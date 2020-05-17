#include "se/utils/Log.h"
#include "se/app/systems/AnimationSystem.h"

namespace se::app {

	AnimationSystem::AnimationSystem(AppComponentDB& appComponentDB, animation::AnimationEngine& animationEngine) :
		EntitySystem(appComponentDB), mAnimationEngine(animationEngine)
	{
		setAccessPolicy<ComponentId::Transforms>(AccessPolicy::Write);
		setAccessPolicy<ComponentId::AnimationNode>(AccessPolicy::Write);
	}


	void AnimationSystem::execute()
	{
		SOMBRA_INFO_LOG << "Updating the AnimationSystem";

		// Update the AnimationNodes with the changes made to the Entities
		/*for (auto itNode = mRootNode->begin(); itNode != mRootNode->end(); ++itNode) {
			auto itNodeEntity = mNodeEntities.find( &(*itNode) );
			if (itNodeEntity != mNodeEntities.end()) {
				Entity* entity = itNodeEntity->second.entity;

				// Reset the Entity animation update
				entity->updated.reset( static_cast<int>(Entity::Update::Animation) );

				if (entity->updated.any()) {
					animation::NodeData& nodeData = itNode->getData();
					animation::AnimationNode* parentNode = itNode->getParent();
					if (parentNode) {
						animation::NodeData& parentData = parentNode->getData();
						nodeData.localTransforms.position = entity->position - parentData.worldTransforms.position;
						nodeData.localTransforms.orientation = glm::inverse(parentData.worldTransforms.orientation) * entity->orientation;
						nodeData.localTransforms.scale = (1.0f / parentData.worldTransforms.scale) * entity->scale;
					}
					else {
						nodeData.localTransforms.position = entity->position;
						nodeData.localTransforms.orientation = entity->orientation;
						nodeData.localTransforms.scale = entity->scale;
					}
					animation::updateWorldTransforms(*itNode);
				}
			}
		}*/

		mAnimationEngine.update(mDeltaTime);

		// Update the TransformsComponents with the changes made to the AnimationNode
		executeForEach([this](AppComponentDB::EntityId entity) {
			const auto& nodeData = getComponentR<ComponentId::AnimationNode>(entity).getData();
			auto& transforms = getComponentW<ComponentId::Transforms>(entity);

			if (nodeData.animated) {
				transforms.position = nodeData.worldTransforms.position;
				transforms.orientation = nodeData.worldTransforms.orientation;
				transforms.scale = nodeData.worldTransforms.scale;
				transforms.updated.set( static_cast<int>(TransformsComponent::Update::Animation) );
			}
		});

		SOMBRA_INFO_LOG << "AnimationSystem updated";
	}

}
