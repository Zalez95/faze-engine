#ifndef COLLISION_DETECTOR_H
#define COLLISION_DETECTOR_H

#include <map>
#include <vector>
#include "Manifold.h"
#include "CoarseCollisionDetector.h"
#include "FineCollisionDetector.h"

namespace collision {

	class Collider;

	/**
	 * Class CollisionDetector
	 */
	class CollisionDetector
	{
	private:	// Nested types
		typedef std::pair<const Collider*, const Collider*> ColliderPair;

	private:	// Attributes
		/** All the Colliders to check */
		std::vector<const Collider*> mColliders;

		/** Maps a pair of Colliders with the the Manifold of it's collision */
		std::map<ColliderPair, Manifold> mMapCollidersManifolds;
		
		/** The cached pointers to all the current contact Manifolds of all
		 * the detected collisions */
		std::vector<Manifold*> mManifolds;

		/** The CoarseCollisionDetector of the CollisionDetector. We will use
		 * it to check what Colliders are intersecting in the broad phase of
		 * the collision detection step */
		CoarseCollisionDetector mCoarseCollisionDetector;

		/** The FineCollisionDetector of the CollisionDetector. We will use
		 * it to generate all the contact data */
		FineCollisionDetector mFineCollisionDetector;

	public:		// Functions
		/** Creates a new CollisionDetector */
		CollisionDetector() {};

		/** Class destructor */
		~CollisionDetector() {};

		/** @return all the contact manifolds of the detected collisions */
		inline std::vector<Manifold*> getCollisionManifolds() const
		{ return mManifolds; };

		/** Adds the given Collider to the CollisionDetector so it will
		 * check if it collides with the other Colliders
		 *
		 * @param	collider a pointer to the Collider that we want to add */
		void addCollider(const Collider* collider);

		/** Removes the given Collider from the CollisionDetector so it wont't
		 * check if it collides with the other Colliders
		 *
		 * @param	collider a pointer to the Collider that we want to
		 * 			remove */
		void removeCollider(const Collider* collider);
		
		/** Calculates all the collisions that can be currently happening
		 * between the Colliders added to the CollisionDetector */
		void update();
	};

}

#endif		// COLLISION_DETECTOR_H