#ifndef FINE_COLLISION_DETECTOR_H
#define FINE_COLLISION_DETECTOR_H

#include <vector>
#include <glm/glm.hpp>
#include "GJKCollisionDetector.h"
#include "EPACollisionDetector.h"

namespace se::collision {

	class Contact;
	class Manifold;
	class Collider;
	class ConvexCollider;
	class ConcaveCollider;


	/**
	 * Class FineCollisionDetector, is the class that calculates the data
	 * generated from the intersection (collision) of volumes (Colliders)
	 */
	class FineCollisionDetector
	{
	private:	// Attributes
		/** The class that implements the GJK algorithm for detecting if two
		 * ConvexColliders are intersecting */
		GJKCollisionDetector mGJKCollisionDetector;

		/** The class that implements the EPA algorithm for calculating the
		 * Contact between the two ConvexColliders that are intersecting */
		EPACollisionDetector mEPACollisionDetector;

		/** The minimum distance between the coordinates of two Contact used
		 * to determine if a contact is the same than other one */
		const float mContactSeparation;

	public:		// Functions
		/** Creates a new FineCollisionDetector
		 *
		 * @param	minFDifference the minimum difference between the distances
		 *			to the origin of two faces needed for the EPA algorithm
		 * @param	contactPrecission the precision of the calculated Contact
		 *			points
		 * @param	contactSeparation the minimum distance between the
		 *			coordinates of two Contact used to determine if a contact
		 *			is the same than another one */
		FineCollisionDetector(
			float minFDifference, float contactPrecision,
			float contactSeparation
		) : mGJKCollisionDetector(contactPrecision),
			mEPACollisionDetector(minFDifference, contactPrecision),
			mContactSeparation(contactSeparation) {};

		/** Checks if the Colliders of the given manifold are intersecting,
		 * and if that is the case, calculates the contact data of the
		 * collision that happened between those Colliders.
		 *
		 * @param	manifold a contact manifold with the Colliders to check if
		 *			they are colliding. If they are colliding, the manifold
		 *			contact data will be updated.
		 * @return	true if the given Colliders are intersecting, false
		 *			otherwise */
		bool collide(Manifold& manifold) const;
	private:
		/** Calculates the contact data of the collision that happened between
		 * the given ConcaveColliders
		 *
		 * @param	collider1 the first of the ConcaveColliders that are
		 *			intersecting
		 * @param	collider2 the second of the ConcaveColliders that are
		 *			intersecting
		 * @param	manifold a contact manifold where the FineCollisionDetector
		 *			will store the collision data
		 * @return	true if the given Colliders are intersecting, false
		 *			otherwise
		 * @note	the colliders must be given in the same order than the ones
		 *			in the manifold */
		bool collideConcave(
			const ConcaveCollider& collider1,
			const ConcaveCollider& collider2,
			Manifold& manifold
		) const;

		/** Calculates the contact data of the collision that happened between
		 * a ConvexCollider and a ConcaveCollider
		 *
		 * @param	convexCollider the ConvexCollider with which we will
		 *			calculate the collision data
		 * @param	concaveCollider the ConcaveCollider with which we will
		 *			calculate the collision data
		 * @param	manifold a contact manifold where the FineCollisionDetector
		 *			will store the collision data
		 * @param	convexFirst if the ConvexCollider is the first collider in
		 *			the Manifold or the second one
		 * @return	true if the given Colliders are intersecting, false
		 *			otherwise */
		bool collideConvexConcave(
			const ConvexCollider& convexCollider,
			const ConcaveCollider& concaveCollider,
			Manifold& manifold, bool convexFirst
		) const;

		/** Calculates the contact data of the collision that happened between
		 * the given ConvexColliders
		 *
		 * @param	collider1 the first ConvexCollider with which we will
		 *			calculate the collision data
		 * @param	collider2 the second ConvexCollider with which we will
		 *			calculate the collision data
		 * @param	manifold a contact manifold where the FineCollisionDetector
		 *			will store the collision data
		 * @return	true if the given Colliders are intersecting, false
		 *			otherwise
		 * @note	the colliders must be given in the same order than the ones
		 *			in the manifold */
		bool collideConvex(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			Manifold& manifold
		) const;

		/** Removes the Contacts that are no longer valid from the given
		 * manifold
		 *
		 * @param	manifold a reference to the Manifold whose contact we want
		 *			to remove */
		void removeInvalidContacts(Manifold& manifold) const;

		/** Checks if the given Contact is close to any of the older Contacts
		 *
		 * @param	newContact the Contact to compare
		 * @param	others the older contacts to compare
		 * @return	true if the newContact is close to any of the older Contacts,
		 *			false otherwise */
		bool isClose(
			const Contact& newContact,
			const std::vector<Contact>& others
		) const;

		/** Limts the number of contacts in the given manifold to 4, leaving
		 * inside the one with the deepest penetration and the 3 most separated
		 * between them
		 *
		 * @param	manifold a reference to the manifold
		 * @return	true if the number of contacs was cut down, false
		 *			otherwise */
		void limitManifoldContacts(Manifold& manifold) const;
	};

}

#endif		// FINE_COLLISION_DETECTOR_H
