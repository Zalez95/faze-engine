#include "se/app/systems/RigidBodySystem.h"
#include "se/utils/Log.h"

namespace se::app {

	RigidBodySystem::RigidBodySystem(AppComponentDB& appComponentDB, physics::PhysicsEngine& physicsEngine) :
		EntitySystem(appComponentDB), mPhysicsEngine(physicsEngine)
	{
		setAccessPolicy<ComponentId::Transforms>(AccessPolicy::Write);
		setAccessPolicy<ComponentId::RigidBody>(AccessPolicy::Write);
	}


	void RigidBodySystem::addEntity(AppComponentDB::EntityId entity)
	{
		executeCallback(entity, [this](AppComponentDB::EntityId entity) {
			if (!hasComponent<ComponentId::Transforms>(entity) && !hasComponent<ComponentId::RigidBody>(entity)) {
				SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added";
				return;
			}

			auto& transforms = getComponentW<ComponentId::Transforms>(entity);
			auto& rigidBody = getComponentW<ComponentId::RigidBody>(entity);

			// The RigidBody initial data is overridden by the entity one
			rigidBody.getData().position		= transforms.position;
			rigidBody.getData().linearVelocity	= transforms.velocity;
			rigidBody.getData().orientation		= transforms.orientation;
			rigidBody.synchWithData();

			// Add the RigidBody
			mPhysicsEngine.addRigidBody(&rigidBody);
			SOMBRA_INFO_LOG << "Entity " << entity << " added successfully";
		});

		EntitySystem::addEntity(entity);
	}


	void RigidBodySystem::removeEntity(AppComponentDB::EntityId entity)
	{
		EntitySystem::removeEntity(entity);

		executeCallback(entity, [this](AppComponentDB::EntityId entity) {
			auto& rigidBody = getComponentW<ComponentId::RigidBody>(entity);
			mPhysicsEngine.removeRigidBody(&rigidBody);
		});
	}

// Private functions
	void RigidBodySystem::execute()
	{
		SOMBRA_INFO_LOG << "Update start";

		mPhysicsEngine.resetRigidBodiesState();

		SOMBRA_DEBUG_LOG << "Updating the RigidBodies";
		executeForEach([this](AppComponentDB::EntityId entity) {
			auto& transforms = getComponentW<ComponentId::Transforms>(entity);
			auto& rigidBody = getComponentW<ComponentId::RigidBody>(entity);

			// Reset the Entity physics update
			transforms.updated.reset( static_cast<int>(TransformsComponent::Update::Physics) );

			if (transforms.updated.any()) {
				rigidBody.getData().position		= transforms.position;
				rigidBody.getData().linearVelocity	= transforms.velocity;
				rigidBody.getData().orientation		= transforms.orientation;
				rigidBody.synchWithData();
			}
		});

		SOMBRA_DEBUG_LOG << "Integrating the RigidBodies";
		mPhysicsEngine.integrate(mDeltaTime);

		SOMBRA_DEBUG_LOG << "Updating the Entities";
		executeForEach([this](AppComponentDB::EntityId entity) {
			auto& transforms = getComponentW<ComponentId::Transforms>(entity);
			auto& rigidBody = getComponentW<ComponentId::RigidBody>(entity);

			if (rigidBody.checkState(physics::RigidBodyState::Integrated)) {
				transforms.position		= rigidBody.getData().position;
				transforms.velocity		= rigidBody.getData().linearVelocity;
				transforms.orientation	= rigidBody.getData().orientation;
				transforms.updated.set( static_cast<int>(TransformsComponent::Update::Physics) );
			}
		});

		SOMBRA_INFO_LOG << "Update end";
	}

}
