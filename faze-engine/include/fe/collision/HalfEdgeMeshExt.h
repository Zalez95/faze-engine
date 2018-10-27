#ifndef HALF_EDGE_MESH_EXT_H
#define HALF_EDGE_MESH_EXT_H

#include <string>
#include "HalfEdgeMesh.h"

namespace fe { namespace collision {

	struct AABB;


	/** Checks if the given HalfEdgeMesh is a valid HalfEdgeMesh (it has
	 * manifold surfaces)
	 *
	 * @param	meshData the HalfEdgeMesh to test
	 * @return	a pair with a bool that indicates whether the given HalfEdgeMesh
	 *			is valid or not and a string that explains the reason it isn't
	 *			valid */
	std::pair<bool, std::string> validateMesh(const HalfEdgeMesh& meshData);


	/** Creates a new HalfEdgeMesh from the given one with its the polygon
	 * HEFaces converted to triangles
	 *
	 * @param	originalMesh the HalfEdgeMesh where the HEFaces are located in
	 * @return	
	 * @note	we will use the ear clipping method, which only works with
	 *			convex polygons (The HEFaces are ensured to be convex if we
	 *			had added them with the addFace method) */
	HalfEdgeMesh triangulateFaces(const HalfEdgeMesh& originalMesh);


	/** Calculates the normal vector of the mesh surface at the given HEVertex
	 *
	 * @note	the normal vector is calculated from the normals of the faces
	 *			that share the vertex without any kind of normalization
	 * @param	meshData the mesh that holds the HEVertex
	 * @param	faceNormals a map with the normals of each HEFace of the mesh
	 * @param	iVertex the index of the HEVertex where we want to calculate
	 *			the normal vector
	 * @return	the normal vector */
	glm::vec3 calculateVertexNormal(
		const HalfEdgeMesh& meshData, const NormalMap& faceNormals,
		int iVertex
	);


	/** Calculates the normal of the given HEFace
	 *
	 * @param	meshData the HalfEdgeMesh where the HEFace is located in
	 * @param	iFace the index of the HEFace
	 * @return	the normal of the HEFace */
	glm::vec3 calculateFaceNormal(const HalfEdgeMesh& meshData, int iFace);


	/** Calculates the Axis Aligned Bounding Box of the given HalfEdgeMesh
	 *
	 * @param	meshData the HalfEdgeMesh to calculate its AABB
	 * @return	the AABB of the HalfEdgeMesh */
	AABB calculateAABB(const HalfEdgeMesh& meshData);


	/** Calculates the furthest point of the HalfEdgeMesh in the given direction
	 * with the Hill-Climbing algorithm
	 *
	 * @param	meshData the HalfEdgeMesh that holds the HEVertices and HEFaces
	 * @param	direction the direction in which we are going to search
	 * @return	the index of the furthest HalfEdgeMesh HEVertex
	 * @note	the HalfEdgeMesh must be convex, otherwise the furthest point
	 *			found could be a local maximum */
	int getFurthestVertexInDirection(
		const HalfEdgeMesh& meshData,
		const glm::vec3& direction
	);


	/** Calculates the boundary of the given HalfEdgeMesh as seen from the given
	 * eye point
	 *
	 * @param	meshData the HalfEdgeMesh that holds the HEVertices and HEFaces
	 * @param	faceNormals the normals of each HEFace
	 * @param	eyePoint the 3D coordinates of the eye point
	 * @param	iInitialFace the index of the initial HEFace from which we will
	 *			start searching
	 * @return	a pair with the list of HEEdge indices that represents the
	 *			boundary of the ConvexHull and the list of HEFace indices with
	 *			the visible HEFaces
	 * @note	the initial HEFace must be visible from the eyePoint
	 *			perspective */
	std::pair<std::vector<int>, std::vector<int>> calculateHorizon(
		const HalfEdgeMesh& meshData, const NormalMap& faceNormals,
		const glm::vec3& eyePoint, int iInitialFace
	);


	/** Calculates the intersection of the given ray with the given mesh
	 *
	 * @param	meshData the convex HalfEdgeMesh to project the point on. The
	 *			points of its faces must lie in the same plane
	 * @param	faceNormals a map with the normals of each HEFace of the
	 *			mesh
	 * @param	origin the origin of the ray. It must be inside of the mesh
	 * @param	direction the direction of the ray
	 * @param	epsilon the epsilon value used for checking if a ray intersects
	 *			a HEFace
	 * @return	a pair with a flag that tells if the ray intersects the
	 *			mesh and the 3D coordinates of the intersection point */
	std::pair<bool, glm::vec3> raycastMesh(
		const HalfEdgeMesh& meshData, const NormalMap& faceNormals,
		const glm::vec3& origin, const glm::vec3& direction,
		float epsilon
	);

}}

#endif		// HALF_EDGE_MESH_EXT_H