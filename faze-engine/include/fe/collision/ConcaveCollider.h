#ifndef CONCAVE_COLLIDER_H
#define CONCAVE_COLLIDER_H

#include <vector>
#include <memory>
#include "Collider.h"

namespace fe { namespace collision {

	class ConvexCollider;


	/**
	 * Class ConcaveCollider, it's a Collider with a Concave shape.
	 */
	class ConcaveCollider : public Collider
	{
	protected:	// Nested types
		typedef std::unique_ptr<ConvexCollider> ConvexPart;

	public:		// Functions
		/** Class destructor */
		virtual ~ConcaveCollider() {};

		/** @return the type of the Collider */
		inline virtual ColliderType getType() const
		{ return ColliderType::CONCAVE_COLLIDER; };

		/** Updates the translation and orientation of the ConcaveCollider
		 * with the data of the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			position and orientation of the ConcaveCollider */
		virtual void setTransforms(const glm::mat4& transforms) = 0;

		/** @return	the transformations matrix currently applied to the
		 *			ConcaveCollider */
		virtual glm::mat4 getTransforms() const = 0;

		/** @return the Axis Aligned Bounding Box that contains the
		 *			ConcaveCollider */
		virtual AABB getAABB() const = 0;

		/** @return a set with the posible overlaping parts of the Collider
		 * with the given AABB
		 *
		 * @param	aabb the AABB to compare
		 * @return	a set with Convex parts of the collider that can be
		 *			overlaping with the given AABB */
		virtual std::vector<ConvexPart> getOverlapingParts(
			const AABB& aabb
		) const = 0;
	};

}}

#endif		// CONCAVE_COLLIDER_H