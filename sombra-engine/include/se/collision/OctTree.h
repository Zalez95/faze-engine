#ifndef OCT_TREE_H
#define OCT_TREE_H

#include <vector>
#include "AABB.h"
#include "Collider.h"

namespace se::collision {

	/**
	 * Class OctTree
	 */
	class OctTree
	{
	private:	// Nested types
		using ColliderPair = std::pair<const Collider*, const Collider*>;

		/** Struct OctTreeNode, it holds all the needed data for a OctTree
		 * node */
		struct OctTreeNode
		{
			/** Pointers to the children nodes of the current node */
			OctTreeNode* children[8];

			/** The position vector of the current node */
			glm::vec3 position;
		};

	private:	// Attributes
		/** The root node of the OctTree */
		OctTreeNode* mRoot;

	public:		// Functions
		/** Creates a new OctTree */
		OctTree() : mRoot(nullptr) {};

		/** Calculates the intersections between Colliders detected by the
		 * OctTree by their AABBs
		 *
		 * @param	limit the maximum number of intersections to calculate
		 * @param	result the output iterator where the indices of the HEFace's
		 *			HEVertices will be stored */
		template <class OutputIterator>
		void getIntersections(int limit, OutputIterator result) const;

		/** Inserts the given Collider as a new node in the OctTree
		 *
		 * @param	collider a pointer to the collider to insert */
		void insert(const Collider* collider);
	private:
		/** Calculates the closest children node index of the given node to
		 * the given position
		 *
		 * @param	node the node that contains the children nodes
		 * @param	position the position
		 * @return	the index of the closest children node in the node children
		 *			array */
		static std::size_t getChildIndex(
			const OctTreeNode& node, const glm::vec3& position
		) {
			int index = 0;
			index += (position.x > node.position.x);
			index += (position.z > node.position.z) << 1;
			index += (position.y > node.position.y) << 2;
			return index;
		};
	};

}

#endif		// OCT_TREE_H
