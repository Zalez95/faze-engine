#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include "se/utils/Log.h"
#include "se/app/systems/CollisionSystem.h"
#include "se/app/events/CollisionEvent2.h"

namespace se::app {

	CollisionSystem::CollisionSystem(
		AppComponentDB& appComponentDB, collision::CollisionWorld& collisionWorld, EventManager& eventManager
	) : EntitySystem(appComponentDB), mCollisionWorld(collisionWorld), mEventManager(eventManager)
	{
		setAccessPolicy<ComponentId::Transforms>(AccessPolicy::Read);
		setAccessPolicy<ComponentId::RigidBody>(AccessPolicy::Write);
	}


	void CollisionSystem::addEntity(AppComponentDB::EntityId entity)
	{
		executeCallback(entity, [this](AppComponentDB::EntityId entity) {
			if (!hasComponent<ComponentId::Transforms>(entity) && !hasComponent<ComponentId::Collider>(entity)) {
				SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added";
				return;
			}

			const auto& transforms = getComponentR<ComponentId::Transforms>(entity);
			auto& collider = getComponentW<ComponentId::Collider>(entity);

			// The Collider initial data is overridden by the entity one
			glm::mat4 translation	= glm::translate(glm::mat4(1.0f), transforms.position);
			glm::mat4 rotation		= glm::mat4_cast(transforms.orientation);
			glm::mat4 scale			= glm::scale(glm::mat4(1.0f), transforms.scale);
			collider->setTransforms(translation * rotation * scale);

			// Add the Collider
			mCollisionWorld.addCollider(collider.get());
			mColliderEntityMap.emplace(collider.get(), entity);

			SOMBRA_INFO_LOG << "Entity " << entity << " added successfully";
		});

		EntitySystem::addEntity(entity);
	}


	void CollisionSystem::removeEntity(AppComponentDB::EntityId entity)
	{
		EntitySystem::removeEntity(entity);

		executeCallback(entity, [this](AppComponentDB::EntityId entity) {
			auto& collider = getComponentW<ComponentId::Collider>(entity);

			auto itPair = mColliderEntityMap.find(collider.get());
			if (itPair != mColliderEntityMap.end()) {
				mCollisionWorld.removeCollider(collider.get());
				mColliderEntityMap.erase(itPair);
				SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
			}
			else {
				SOMBRA_WARN_LOG << "Entity " << entity << " wasn't removed";
			}
		});
	}


	std::vector<CollisionSystem::EntityRayCastPair> CollisionSystem::getEntities(
		const glm::vec3& rayOrigin, const glm::vec3& rayDirection
	) const
	{
		SOMBRA_INFO_LOG << "Performing rayCast from "
			<< glm::to_string(rayOrigin) << " towards " << glm::to_string(rayDirection);

		std::vector<CollisionSystem::EntityRayCastPair> ret;

		mCollisionWorld.processRayCast(
			rayOrigin, rayDirection,
			[&](const collision::Collider& collider, const collision::RayCast& rayCast) {
				auto it = mColliderEntityMap.find(&collider);
				if (it != mColliderEntityMap.end()) {
					SOMBRA_DEBUG_LOG << "RayCast against Entity " << it->second << " OK";
					ret.emplace_back(it->second, rayCast);
				}
			}
		);

		SOMBRA_INFO_LOG << "RayCast finished with " << ret.size() << " entities";
		return ret;
	}

// Private functions
	void CollisionSystem::execute()
	{
		SOMBRA_INFO_LOG << "Updating the CollisionSystem";

		SOMBRA_DEBUG_LOG << "Updating Colliders";
		executeForEach([this](AppComponentDB::EntityId entity) {
			const auto& transforms = getComponentW<ComponentId::Transforms>(entity);
			auto& collider = getComponentW<ComponentId::Collider>(entity);

			if (transforms.updated.any()) {
				glm::mat4 translation	= glm::translate(glm::mat4(1.0f), transforms.position);
				glm::mat4 rotation		= glm::mat4_cast(transforms.orientation);
				glm::mat4 scale			= glm::scale(glm::mat4(1.0f), transforms.scale);
				collider->setTransforms(translation * rotation * scale);
			}
		});

		SOMBRA_DEBUG_LOG << "Detecting collisions between the colliders";
		mCollisionWorld.update();

		SOMBRA_DEBUG_LOG << "Notifying contact manifolds";
		mCollisionWorld.processCollisionManifolds([this](const collision::Manifold& manifold) {
			auto itPair1 = mColliderEntityMap.find(manifold.colliders[0]);
			auto itPair2 = mColliderEntityMap.find(manifold.colliders[1]);
			if ((itPair1 != mColliderEntityMap.end()) && (itPair2 != mColliderEntityMap.end())
				&& manifold.state[collision::Manifold::State::Updated]
			) {
				auto event = new CollisionEvent2(itPair1->second, itPair2->second, &manifold);
				SOMBRA_DEBUG_LOG << "Notifying new CollisionEvent " << *event;
				mEventManager.publish(event);
			}
		});

		SOMBRA_INFO_LOG << "CollisionSystem updated";
	}

}
