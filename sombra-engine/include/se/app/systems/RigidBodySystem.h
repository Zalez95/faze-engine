#ifndef RIGID_BODY_SYSTEM_H
#define RIGID_BODY_SYSTEM_H

#include "../../physics/PhysicsEngine.h"
#include "EntitySystem.h"

namespace se::app {

	/**
	 * Class RigidBodySystem, it's a Manager used for updating the Entities'
	 * RigidBodies
	 */
	class RigidBodySystem : public EntitySystem
	{
	private:	// Attributes
		/** The PhysicsEngine used for updating the RigidBodies */
		physics::PhysicsEngine& mPhysicsEngine;

	public:		// Functions
		/** Creates a new RigidBodySystem
		 *
		 * @param	appComponentDB a reference to the AppComponentDB that holds
		 *			all the Component of the Entities
		 * @param	physicsEngine a reference to the PhysicsEngine used for
		 *			updating the RigidBodies */
		RigidBodySystem(
			AppComponentDB& appComponentDB,
			physics::PhysicsEngine& physicsEngine
		);

		/** Adds the given Entity to the current RigidBodySystem
		 *
		 * @param	entity the Entity to add to the RigidBodySystem */
		virtual void addEntity(AppComponentDB::EntityId entity) override;

		/** Removes the given Entity from the current RigidBodySystem
		 *
		 * @param	entity the Entity to remove from the RigidBodySystem */
		virtual void removeEntity(AppComponentDB::EntityId entity) override;
	private:
		/** Synchs the RigidBodies location with the Entities location */
		virtual void execute() override;
	};

}

#endif		// RIGID_BODY_SYSTEM_H
