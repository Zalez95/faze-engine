#ifndef CONCAVE_COLLIDER_H
#define CONCAVE_COLLIDER_H

#include <vector>
#include <memory>
#include "Collider.h"

namespace se::collision {

	class ConvexCollider;


	/**
	 * Class ConcaveCollider, it's a Collider with a Concave shape.
	 */
	class ConcaveCollider : public Collider
	{
	public:		// Functions
		/** Updates the scale, translation and orientation of the
		 * ConcaveCollider with the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			scale, translation and orientation of the ConcaveCollider */
		void setTransforms(const glm::mat4& transforms) override = 0;

		/** @return	the transformations matrix currently applied to the
		 *			ConcaveCollider */
		glm::mat4 getTransforms() const override = 0;

		/** @return the Axis Aligned Bounding Box that contains the
		 *			ConcaveCollider */
		AABB getAABB() const override = 0;

		/** @return a set with the posible overlaping parts of the Collider
		 * with the given AABB
		 *
		 * @param	aabb the AABB to compare
		 * @return	a set with the pointers to the Convex parts of the collider
		 *			that could be overlaping with the given AABB */
		virtual std::vector<const ConvexCollider*> getOverlapingParts(
			const AABB& aabb
		) const = 0;
	};

}

#endif		// CONCAVE_COLLIDER_H
