#ifndef COLLISION_DETECTOR_H
#define COLLISION_DETECTOR_H

#include <vector>
#include "Contact.h"

namespace physics {

	class Collider;
	class Plane;
	class BoundingSphere;
	class AxisAlignedBoundingBox;

	/**
	 * Class CollisionDetector, is the class that calculates the CollisionData
	 * generated from the collision of the Colliders
	 */
	class CollisionDetector
	{
	public:		// Functions
		/** Creates a new CollisionDetector */
		CollisionDetector() {};

		/** Class destructor */
		~CollisionDetector() {};

		/** Returns the data of the collision happened between the given
		 * Colliders
		 * 
		 * @param	collider1 a pointer to the first Collider with which we 
		 *			will calculate the collision data
		 * @param	collider2 a pointer to the second Collider with which we
		 *			will calculate the collision data
		 * @return	the data of the Collision */
		std::vector<Contact> collide(
			const Collider* collider1,
			const Collider* collider2
		) const;
	private:
		/** Returns the data of the collision happened between the given
		 * BoundingSpheres
		 *  
		 * @param	sphere1 a pointer to the first BoundingSphere with which
		 *			we will calculate the data of the collision
		 * @param	sphere2 a pointer to the second BoundingSphere with which
		 *			we will calculate the data of the collision
		 * @return	the data of the collision */
		std::vector<Contact> collideSpheres(
			const BoundingSphere* sphere1,
			const BoundingSphere* sphere2
		) const;

		/** Returns the data of the collision happened between the given
		 * sphere and the given plane
		 * 
		 * @note	the BoundingSphere can collide with the plane only if it
		 *			crosses the plane in the opposite direction of the
		 *			plane's normal
		 * @param	sphere a pointer to the BoundingSphere with which we will
		 *			calculate the data of the collision
		 * @param	plane a pointer to the plane with which we will calculate
		 *			the data of the collision
		 * @return	the data of the collision */
		std::vector<Contact> collideSphereAndPlane(
			const BoundingSphere* sphere,
			const Plane* plane
		) const;

		/** Returns the data of the collision happened between the given
		* AABBs
		*
		* @param	aabb1 a pointer to the first AxisAlignedBoundingBox with
		*			which we will calculate the data of the collision
		* @param	aabb2 a pointer to the second AxisAlignedBoundingBox with
		*			which we will calculate the data of the collision
		* @return	the data of the collision */
		std::vector<Contact> collideAABBs(
			const AxisAlignedBoundingBox* aabb1,
			const AxisAlignedBoundingBox* aabb2
		) const;
		
		/** Returns the data of the collision happened between the given
		 * AABB and the given plane
		 * 
		 * @note	the AABB can collide with the plane only if it crosses the
		 * 			plane in the opposite direction of the plane's normal
		 * @param	aabb a pointer to the AxisAlignedBoundingBox with which we
		 *			will calculate the data of the collision
		 * @param	plane a pointer to the plane with which we will calculate
		 *			the data of the collision
		 * @return	the data of the collision */
		std::vector<Contact> collideAABBAndPlane(
			const AxisAlignedBoundingBox* aabb,
			const Plane* plane
		) const;

		/** Returns the data of the collision happened between the given
		 * AABB and the given Sphere
		 *  
		 * @param	sphere a pointer to the BoundingSphere with which we will
		 *			calculate the data of the collision
		 * @param	aabb a pointer to the AxisAlignedBoundingBox with which we
		 *			will calculate the data of the collision
		 * @return	the data of the collision */
		std::vector<Contact> collideSphereAndAABB(
			const BoundingSphere* sphere,
			const AxisAlignedBoundingBox* aabb
		) const;
	};

}

#endif		// COLLISION_DETECTOR_H
