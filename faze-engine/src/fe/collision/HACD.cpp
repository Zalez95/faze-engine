#include <tuple>
#include <limits>
#include <algorithm>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/constants.hpp>
#include "fe/collision/HACD.h"
#include "fe/collision/QuickHull.h"
#include "fe/collision/HalfEdgeMeshExt.h"
#include "Geometry.h"

namespace fe { namespace collision {

	void HACD::calculate(const HalfEdgeMesh& originalMesh)
	{
		initData(originalMesh);

		// Create a Queue of Graph Edges to collapse ordered by its cost
		// from highest to lowest
		std::vector<QHACDData> vertexPairsByCost;
		for (const DualGraphVertex& vertex1 : mDualGraph.vertices) {
			for (int iVertex2 : vertex1.neighbours) {
				auto itVertex2 = std::lower_bound(mDualGraph.vertices.begin(), mDualGraph.vertices.end(), iVertex2);

				// We filter the neighbour vertices already evaluated
				if ((itVertex2 != mDualGraph.vertices.end()) && (itVertex2->id > vertex1.id)) {
					QHACDData curData = createQHACDData(vertex1, *itVertex2);
					vertexPairsByCost.insert(
						std::lower_bound(vertexPairsByCost.begin(), vertexPairsByCost.end(), curData, std::greater<QHACDData>()),
						curData
					);
				}
			}
		}

		// Collapse the Graph Edge with the lowest cost until there's no one
		// unther the concavity threshold
		while(std::any_of(
				vertexPairsByCost.begin(), vertexPairsByCost.end(),
				[this](const QHACDData& qd) { return qd.concavity < mMaximumConcavity * mNormalizationFactor; }
			)
		) {
			QHACDData curData = vertexPairsByCost.back();
			vertexPairsByCost.pop_back();

			auto itVertex1 = std::lower_bound(mDualGraph.vertices.begin(), mDualGraph.vertices.end(), curData.iVertex1);
			auto itVertex2 = std::lower_bound(mDualGraph.vertices.begin(), mDualGraph.vertices.end(), curData.iVertex2);

			// 1. Update the ancestors of the first vertex with the second one
			updateAncestors(*itVertex1, *itVertex2);

			// 2. Merge both nodes into the first one
			halfEdgeCollapse(itVertex1->id, itVertex2->id, mDualGraph);

			// 3. Remove all the elements of the Queue that holds the Vertex 1
			// or 2
			vertexPairsByCost.erase(
				std::remove_if(
					vertexPairsByCost.begin(), vertexPairsByCost.end(),
					[&](const QHACDData& other) { return curData.compareVertexIds(other); }
				),
				vertexPairsByCost.end()
			);

			// 4. Add new elements to the Queue with the updated vertex 1 data
			itVertex1 = std::lower_bound(mDualGraph.vertices.begin(), mDualGraph.vertices.end(), curData.iVertex1);
			for (int iVertex2 : itVertex1->neighbours) {
				itVertex2 = std::lower_bound(mDualGraph.vertices.begin(), mDualGraph.vertices.end(), iVertex2);

				curData = createQHACDData(*itVertex1, *itVertex2);
				vertexPairsByCost.insert(
					std::lower_bound(vertexPairsByCost.begin(), vertexPairsByCost.end(), curData, std::greater<QHACDData>()),
					curData
				);
			}
		}

		computeConvexSurfaces();
	}


	void HACD::resetData()
	{
		mFaceNormals.clear();
		mConvexMeshes.clear();
	}

// Private functions
	void HACD::initData(const HalfEdgeMesh& originalMesh)
	{
		// 1. Calculate the triangulated mesh
		mMesh = triangulateFaces(originalMesh);

		// 2. Calculate the face normals of the mesh
		for (auto it = mMesh.faces.begin(); it != mMesh.faces.end(); ++it) {
			mFaceNormals.emplace(it.getIndex(), calculateFaceNormal(mMesh, it.getIndex()));
		}

		// 3. Calculate the initial dual graph of the triangulated mesh
		mDualGraph = createDualGraph(mMesh);

		// 4. Calculate the AABB of the mesh
		AABB meshAABB = calculateAABB(mMesh);

		// 5. Calculate the normalization factor of the triangulated mesh
		mNormalizationFactor = calculateNormalizationFactor(meshAABB);

		// 6. Calculate the scaled epsilon value
		mScaledEpsilon = mNormalizationFactor * mEpsilon;

		// 7. Calculate the aspect ratio factor of the triangulated mesh
		mAspectRatioFactor = calculateAspectRatioFactor(mNormalizationFactor);
	}


	HACD::QHACDData HACD::createQHACDData(
		const DualGraphVertex& vertex1, const DualGraphVertex& vertex2
	) const
	{
		// Calculate the surface created from the current vertices and
		// their ancestors
		HalfEdgeMesh surface;
		NormalMap surfaceNormals;
		auto surfaceFaceIndices = calculateSurfaceFaceIndices(vertex1, vertex2);
		std::tie(surface, surfaceNormals) = getMeshFromIndices(surfaceFaceIndices, mMesh, mFaceNormals);

		// Calculate the cost of the surface
		QuickHull qh(mEpsilon);
		qh.calculate(surface);
		float concavity = calculateConcavity(surface, surfaceNormals, qh.getMesh(), qh.getNormalsMap());
		float aspectRatio = calculateAspectRatio(surface);
		float cost = calculateDecimationCost(concavity, aspectRatio);

		return { vertex1.id, vertex2.id, cost, concavity };
	}


	void HACD::updateAncestors(
		DualGraphVertex& vertex1, const DualGraphVertex& vertex2
	) {
		std::vector<int> joinedAncestors;
		std::set_union(
			vertex1.data.begin(), vertex1.data.end(),
			vertex2.data.begin(), vertex2.data.end(),
			std::back_inserter(joinedAncestors)
		);

		vertex1.data = joinedAncestors;
		vertex1.data.insert(
			std::lower_bound(vertex1.data.begin(), vertex1.data.end(), vertex2.id),
			vertex2.id
		);
	}


	void HACD::computeConvexSurfaces()
	{
		QuickHull qh(mEpsilon);

		mConvexMeshes.reserve(mDualGraph.vertices.size());
		for (auto graphVertex : mDualGraph.vertices) {
			std::vector<int> iFaces = { graphVertex.id };
			iFaces.insert(iFaces.end(), graphVertex.data.begin(), graphVertex.data.end());

			// Create a surface from the current vertex and its ancestors
			HalfEdgeMesh surface;
			std::map<int, int> vertexIndexMap;
			for (int iFace : iFaces) {
				// Add the HEVertices to the surface if they aren't already in
				// it
				std::vector<int> surfaceFaceIndices;
				for (int iMeshVertex : getFaceIndices(mMesh, iFace)) {
					int iSurfaceVertex = -1;

					auto itVertexIndex = vertexIndexMap.find(iMeshVertex);
					if (itVertexIndex != vertexIndexMap.end()) {
						iSurfaceVertex = itVertexIndex->second;
					}
					else {
						glm::vec3 vertexLocation = mMesh.vertices[iMeshVertex].location;
						iSurfaceVertex = addVertex(surface, vertexLocation);
						vertexIndexMap.emplace(iMeshVertex, iSurfaceVertex);
					}

					surfaceFaceIndices.push_back(iSurfaceVertex);
				}

				// Add the HEFace to the surface
				addFace(surface, surfaceFaceIndices);
			}

			// Push the convex hull of the surface to the convex surfaces vector
			qh.resetData();
			qh.calculate(surface);
			mConvexMeshes.push_back(qh.getMesh());
		}
	}


	HACD::DualGraph HACD::createDualGraph(const HalfEdgeMesh& meshData) const
	{
		DualGraph dualGraph;

		// Create the dual graph from the Mesh HEFaces
		for (auto itFace = meshData.faces.begin(); itFace != meshData.faces.end(); ++itFace) {
			dualGraph.vertices.emplace_back(itFace.getIndex(), std::vector<int>());
		}
		std::sort(dualGraph.vertices.begin(), dualGraph.vertices.end());

		// Create the vertices neighbours from the Mesh adjacent HEFaces
		for (DualGraphVertex& v : dualGraph.vertices) {
			int iInitialEdge = meshData.faces[v.id].edge;
			int iCurrentEdge = iInitialEdge;
			do {
				HEEdge currentEdge = meshData.edges[iCurrentEdge];
				HEEdge oppositeEdge = meshData.edges[currentEdge.oppositeEdge];

				int iOtherVertex = oppositeEdge.face;
				auto itOtherVertex = std::lower_bound(dualGraph.vertices.begin(), dualGraph.vertices.end(), iOtherVertex);

				// Check if we already set the other vertex as a neighbour of
				// the current one
				if ((itOtherVertex != dualGraph.vertices.end())
					&& !std::binary_search(v.neighbours.begin(), v.neighbours.end(), iOtherVertex)
				) {
					// Set the other vertex as a neighbour of the current one
					v.neighbours.insert(
						std::lower_bound(v.neighbours.begin(), v.neighbours.end(), iOtherVertex),
						iOtherVertex
					);

					// Set the current vertex as a neighbour of the other one
					itOtherVertex->neighbours.insert(
						std::lower_bound(itOtherVertex->neighbours.begin(), itOtherVertex->neighbours.end(), v.id),
						v.id
					);
				}

				iCurrentEdge = currentEdge.nextEdge;
			}
			while (iCurrentEdge != iInitialEdge);
		}

		return dualGraph;
	}


	float HACD::calculateNormalizationFactor(const AABB& aabb)
	{
		return glm::length(aabb.maximum - aabb.minimum);
	}


	float HACD::calculateAspectRatioFactor(float normalizationFactor) const
	{
		return mMaximumConcavity / (10.0f * normalizationFactor);
	}


	std::vector<int> HACD::calculateSurfaceFaceIndices(
		const DualGraphVertex& vertex1, const DualGraphVertex& vertex2
	) {
		std::vector<int> surfaceFaceIndices = { vertex1.id, vertex2.id };
		surfaceFaceIndices.insert(surfaceFaceIndices.end(), vertex1.data.begin(), vertex1.data.end());
		surfaceFaceIndices.insert(surfaceFaceIndices.end(), vertex2.data.begin(), vertex2.data.end());

		return surfaceFaceIndices;
	}


	std::pair<HalfEdgeMesh, NormalMap> HACD::getMeshFromIndices(
		const std::vector<int>& iFaces,
		const HalfEdgeMesh& meshData, const NormalMap& faceNormals
	) {
		HalfEdgeMesh newMesh;
		NormalMap newMeshNormals;

		std::map<int, int> vertexMap;
		for (int iFace1 : iFaces) {
			std::vector<int> iFace1Vertices = getFaceIndices(meshData, iFace1);
			std::vector<int> iFace2Vertices;
			for (int iVertex1 : iFace1Vertices) {
				auto itVertex1 = vertexMap.find(iVertex1);
				if (itVertex1 != vertexMap.end()) {
					iFace2Vertices.push_back(itVertex1->second);
				}
				else {
					int iVertex2 = addVertex(newMesh, meshData.vertices[iVertex1].location);
					vertexMap.emplace(iVertex1, iVertex2);
					iFace2Vertices.push_back(iVertex2);
				}
			}

			int iFace2 = addFace(newMesh, iFace2Vertices);
			newMeshNormals.emplace(iFace2, faceNormals.at(iFace1));
		}

		return std::make_pair(newMesh, newMeshNormals);
	}


	float HACD::calculateConcavity(
		const HalfEdgeMesh& originalMesh, const NormalMap& faceNormals,
		const HalfEdgeMesh& convexHullMesh, const NormalMap& convexHullNormals
	) const
	{
		float concavity = 0.0f;

		glm::vec3 polygonNormal = (convexHullNormals.empty())? glm::vec3(0.0f) : convexHullNormals.begin()->second;
		if (std::all_of(
				convexHullNormals.begin(), convexHullNormals.end(),
				[&](const std::pair<int, glm::vec3>& pair) {
					return glm::all(glm::epsilonEqual(pair.second, polygonNormal, mScaledEpsilon));
				}
			)
		) {
			HalfEdgeMesh triangulatedConvexHullMesh = triangulateFaces(convexHullMesh);
			concavity = calculateConcavity2D(originalMesh, triangulatedConvexHullMesh);
		}
		else {
			concavity = calculateConcavity3D(originalMesh, faceNormals, convexHullMesh, convexHullNormals);
		}

		return concavity;
	}


	float HACD::calculateConcavity2D(const HalfEdgeMesh& originalMesh, const HalfEdgeMesh& convexHullMesh) const
	{
		float originalMeshArea = 0.0f;
		for (auto itFace = originalMesh.faces.begin(); itFace != originalMesh.faces.end(); ++itFace) {
			auto faceIndices = getFaceIndices(originalMesh, itFace.getIndex());
			originalMeshArea += calculateTriangleArea({
				originalMesh.vertices[faceIndices[0]].location,
				originalMesh.vertices[faceIndices[1]].location,
				originalMesh.vertices[faceIndices[2]].location
			});
		}

		float convexHullArea = 0.0f;
		for (auto itFace = convexHullMesh.faces.begin(); itFace != convexHullMesh.faces.end(); ++itFace) {
			auto faceIndices = getFaceIndices(convexHullMesh, itFace.getIndex());
			convexHullArea += calculateTriangleArea({
				convexHullMesh.vertices[faceIndices[0]].location,
				convexHullMesh.vertices[faceIndices[1]].location,
				convexHullMesh.vertices[faceIndices[2]].location
			});
		}

		return std::sqrt(convexHullArea - originalMeshArea);
	}


	float HACD::calculateConcavity3D(
		const HalfEdgeMesh& originalMesh, const NormalMap& faceNormals,
		const HalfEdgeMesh& convexHullMesh, const NormalMap& convexHullNormals
	) const
	{
		float maxConcavity = -std::numeric_limits<float>::max();
		for (auto itVertex = originalMesh.vertices.begin(); itVertex != originalMesh.vertices.end(); ++itVertex) {
			glm::vec3 vertexLocation = itVertex->location;
			glm::vec3 vertexNormal = calculateVertexNormal(originalMesh, faceNormals, itVertex.getIndex());

			bool intersects;
			glm::vec3 intersection;
			std::tie(intersects, intersection) = getInternalIntersection(convexHullMesh, convexHullNormals, vertexLocation, vertexNormal);
			if (intersects) {
				float currentConcavity = glm::length(intersection - vertexLocation);
				maxConcavity = std::max(maxConcavity, currentConcavity);
			}
		}

		return maxConcavity;
	}


	float HACD::calculateAspectRatio(const HalfEdgeMesh& meshData)
	{
		// 1. Calculate the perimeter of the surface of the triangles
		float perimeter = 0.0f;
		for (auto itFace = meshData.faces.begin(); itFace != meshData.faces.end(); ++itFace) {
			int iInitialEdge = itFace->edge;
			int iCurrentEdge = iInitialEdge;
			do {
				const HEEdge& currentEdge = meshData.edges[iCurrentEdge];
				const HEEdge& oppositeEdge = meshData.edges[currentEdge.oppositeEdge];
				if (!meshData.faces.isActive(oppositeEdge.face)) {
					glm::vec3 sharedV1 = meshData.vertices[oppositeEdge.vertex].location;
					glm::vec3 sharedV2 = meshData.vertices[currentEdge.vertex].location;
					perimeter += glm::length(sharedV2 - sharedV1);
				}
				iCurrentEdge = currentEdge.nextEdge;
			}
			while (iCurrentEdge != iInitialEdge);
		}

		// 2. Calculate the area of the surface as the sum of the areas of the
		// triangles
		float area = 0.0f;
		for (auto itFace = meshData.faces.begin(); itFace != meshData.faces.end(); ++itFace) {
			auto faceIndices = getFaceIndices(meshData, itFace.getIndex());
			area += calculateTriangleArea({
				meshData.vertices[faceIndices[0]].location,
				meshData.vertices[faceIndices[1]].location,
				meshData.vertices[faceIndices[2]].location
			});
		}

		return std::pow(perimeter, 2) / (4 * glm::pi<float>() * area);
	}


	float HACD::calculateDecimationCost(float concavity, float aspectRatio) const
	{
		return concavity / mNormalizationFactor + mAspectRatioFactor * aspectRatio;
	}


	std::pair<bool, glm::vec3> HACD::getInternalIntersection(
		const HalfEdgeMesh& meshData, const NormalMap& faceNormals,
		const glm::vec3& origin, const glm::vec3& direction
	) const
	{
		bool intersects1 = false, intersects2 = false;
		glm::vec3 intersection1, intersection2, face1Point, face2Point, face1Normal, face2Normal;
		auto itFace1 = meshData.faces.begin(), itFace2 = itFace1;

		// Search one intersected HEFaces
		for (; (itFace1 != meshData.faces.end()) && !intersects1; ++itFace1) {
			face1Point = meshData.vertices[ meshData.edges[itFace1->edge].vertex ].location;
			face1Normal = faceNormals.at( itFace1.getIndex() );

			std::tie(intersects1, intersection1) = projectPointInDirection(origin, direction, face1Point, face1Normal);
			if (intersects1) {
				intersects1 = isPointBetweenHEEdges(meshData, itFace1->edge, face1Normal, intersection1);
			}
		}

		// Check if the origin is on the same plane than the first HEFace
		if (intersects1
			&& (itFace1 != meshData.faces.end())
			&& (glm::dot(origin - face1Point, face1Normal) < mScaledEpsilon)
		) {
			// Search another intersection point (a convex mesh has at most
			// two intersections)
			for (itFace2 = ++itFace1; (itFace2 != meshData.faces.end()) && !intersects2; ++itFace2) {
				face2Point = meshData.vertices[ meshData.edges[itFace2->edge].vertex ].location;
				face2Normal = faceNormals.at( itFace2.getIndex() );

				std::tie(intersects2, intersection2) = projectPointInDirection(origin, direction, face2Point, face2Normal);
				if (intersects2) {
					intersects2 = isPointBetweenHEEdges(meshData, itFace2->edge, face2Normal, intersection2);

					// Check if the intersection point is the same than the
					// first one
					if ( glm::all(glm::epsilonEqual(intersection1, intersection2, mScaledEpsilon)) ) {
						intersects2 = false;
					}
				}
			}
		}

		// Return the furthest intersection point from the origin
		float length1 = (intersects1)? glm::length(intersection1 - origin) : -std::numeric_limits<float>::max();
		float length2 = (intersects2)? glm::length(intersection2 - origin) : -std::numeric_limits<float>::max();
		return std::make_pair(intersects1, (length1 > length2)? intersection1 : intersection2);
	}


	bool HACD::isPointBetweenHEEdges(
		const HalfEdgeMesh& meshData, int iInitialEdge,
		const glm::vec3& loopNormal, const glm::vec3& point
	) const
	{
		bool inside = true;

		int iCurrentEdge = iInitialEdge;
		do {
			const HEEdge& currentEdge = meshData.edges[iCurrentEdge];
			const HEEdge& oppositeEdge = meshData.edges[currentEdge.oppositeEdge];

			glm::vec3 p1 = meshData.vertices[oppositeEdge.vertex].location;
			glm::vec3 p2 = meshData.vertices[currentEdge.vertex].location;
			if (glm::dot(glm::cross(p2 - p1, loopNormal), point - p1) > mScaledEpsilon) {
				inside = false;
			}

			iCurrentEdge = currentEdge.nextEdge;
		}
		while ((iCurrentEdge != iInitialEdge) && inside);

		return inside;
	}

}}
