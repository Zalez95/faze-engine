#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include <unordered_map>
#include "../../collision/CollisionWorld.h"
#include "../events/EventManager.h"
#include "EntitySystem.h"

namespace se::app {

	/**
	 * Class CollisionSystem, it's a Manager used for updating the Entities'
	 * Colliders
	 */
	class CollisionSystem : public EntitySystem
	{
	private:	// Nested types
		using EntityRayCastPair = std::pair<
			AppComponentDB::EntityId, collision::RayCast
		>;

	private:	// Attributes
		/** The CollisionWorld used for updating the Colliders */
		collision::CollisionWorld& mCollisionWorld;

		/** The EventManager used for notifying events */
		EventManager& mEventManager;

		/** Maps the Colliders added to the CollisionManager and its Entities */
		std::unordered_map<const collision::Collider*, AppComponentDB::EntityId>
			mColliderEntityMap;

	public:		// Functions
		/** Creates a new CollisionSystem
		 *
		 * @param	appComponentDB a reference to the AppComponentDB that holds
		 *			all the Component of the Entities
		 * @param	collisionWorld a reference to the CollisionWorld used for
		 *			updating the Colliders
		 * @param	eventManager a reference to the EventManager that the
		 *			CollisionManager will use to notify the detected
		 *			collisions */
		CollisionSystem(
			AppComponentDB& appComponentDB,
			collision::CollisionWorld& collisionWorld,
			EventManager& eventManager
		);

		/** Adds the given Entity to the current CollisionSystem
		 *
		 * @param	entity the Entity to add to the CollisionSystem */
		virtual void addEntity(AppComponentDB::EntityId entity) override;

		/** Removes the given Entity from the current CollisionSystem
		 *
		 * @param	entity the Entity to remove from the CollisionSystem */
		virtual void removeEntity(AppComponentDB::EntityId entity) override;

		std::vector<EntityRayCastPair> getEntities(
			const glm::vec3& rayOrigin, const glm::vec3& rayDirection
		) const;
	private:
		/** Synchs the Colliders location with the Entities location */
		virtual void execute() override;
	};

}

#endif		// COLLISION_SYSTEM_H
