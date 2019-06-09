#ifndef HALF_EDGE_MESH_RAYCAST_H
#define HALF_EDGE_MESH_RAYCAST_H

#include "AABB.h"
#include "HalfEdgeMesh.h"

namespace se::collision {

	/**
	 * Struct RayHit, holds the ray intersection data
	 */
	struct RayHit
	{
		/** If the Ray intersects the HEMesh or doesn't */
		bool intersects;

		/** The index of the intersected HEFace in the HEMesh */
		int iFace;

		/** The intersection point of the ray with the HEMesh */
		glm::vec3 intersection;

		/** The distance of the ray origin to the intersection point */
		float distance;
	};


	/**
	 * Class HalfEdgeMeshRaycast, it's used to Ray-Mesh intersections with
	 * the HalfEdgeMesh objects
	 */
	class HalfEdgeMeshRaycast
	{
	private:	// Nested types
		/** Holds the data of each node in the kd-tree */
		struct TreeNode
		{
			/** The indices of the HEFaces of the current node */
			std::vector<int> iFaces;

			/** The bounding box that contains all of the current HEFaces */
			AABB aabb;

			/** The index of our left child node in the kd-tree */
			int iLeftChild;

			/** The index of our right child node in the kd-tree */
			int iRightChild;
		};

		/** A kd-tree used for splitting the HEFaces, the tree structure is
		 * stored inside a vector */
		using Tree = std::vector<TreeNode>;

	private:	// Attributes
		/** The HalfEdgeMesh to calculate the ray intersections with */
		const HalfEdgeMesh& mMesh;

		/** The normal vectors of the HalfEdgeMesh HEFaces */
		const ContiguousVector<glm::vec3>& mFaceNormals;

		/** The epsilon value needed for the comparisons with the half rays */
		const float mEpsilon;

		/** The maximum depth of the generated binary-tree */
		const int mMaxDepth;

		/** The binary tree that will be used for splitting the mesh HEFaces
		 * and store their respective bounding spheres */
		Tree mKDTree;

		/** The index of the root node of the kd-tree */
		int mIRootNode;

	public:		// Functions
		/** Creates a new HalfEdgeMeshRaycast object
		 *
		 * @param	mesh the Half-Edge data structure with the 3D Mesh
		 *			to calculate the Raycasts
		 * @param	faceNormals the normal vectors of the mesh HEFaces
		 * @param	epsilon the epsilon value needed for the comparisons with
		 *			the half rays
		 * @param	maxDepth the maximum depth of the generated kd-tree */
		HalfEdgeMeshRaycast(
			const HalfEdgeMesh& mesh,
			const ContiguousVector<glm::vec3>& faceNormals,
			float epsilon, int maxDepth
		);

		/** Calculates the closest ray-mesh intersection iteratively
		 *
		 * @param	rayOrigin the coordinates of the origin of the ray
		 * @param	rayDirection the direction of the ray
		 * @return	the intersection data */
		RayHit closestHit(
			const glm::vec3& rayOrigin, const glm::vec3& rayDirection
		) const;
	private:
		/** Builds the kd-tree iteratively */
		void buildKDTree();

		/** Calculates the AABB of the given HEFaces of the current HEMesh
		 *
		 * @param	faceIndices the indices of the HEFaces in the HEMesh
		 * @return	the calculated AABB */
		AABB calculateAABBFromFaces(const std::vector<int>& faceIndices) const;

		/** Checks if the given point is located between the given loop edges
		 *
		 * @param	meshData the HalfEdgeMesh where the HEEdges are located
		 * @param	iInitialEdge the index of the starting HEEdge of the loop
		 * @param	loopNormal the normal vector of the HEEdge loop
		 * @param	point the coordinates of the point to check
		 * @return	true if the point is between the HEEdges, false otherwise */
		bool isPointBetweenHEEdges(
			const HalfEdgeMesh& meshData, int iInitialEdge,
			const glm::vec3& loopNormal, const glm::vec3& point
		) const;
	};

}

#endif		// HALF_EDGE_MESH_RAYCAST_H
