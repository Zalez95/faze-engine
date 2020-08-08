#include "se/utils/Log.h"
#include "se/graphics/Technique.h"
#include "se/graphics/Pass.h"
#include "se/app/RMeshSystem.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"
#include "se/app/graphics/Skin.h"

namespace se::app {

	void RMeshSystem::onNewEntity(Entity entity)
	{
		auto [transforms, rMesh, skin] = mEntityDatabase.getComponents<TransformsComponent, graphics::RenderableMesh, Skin>(entity);
		if (!rMesh) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Mesh";
			return;
		}

		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), transforms->position);
		glm::mat4 rotation		= glm::mat4_cast(transforms->orientation);
		glm::mat4 scale			= glm::scale(glm::mat4(1.0f), transforms->scale);
		glm::mat4 modelMatrix	= translation * rotation * scale;

		auto& meshData = mRenderableMeshEntities[entity];
		rMesh->processTechniques([&, rMesh = rMesh, skin = skin](auto technique) { technique->processPasses([&](auto pass) {
			auto it = std::find_if(
				mCameraSystem.mPassesData.begin(), mCameraSystem.mPassesData.end(),
				[&](const auto& passData) { return passData.pass == pass; }
			);
			if (it != mCameraSystem.mPassesData.end()) {
				meshData.modelMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *it->program, modelMatrix);
				rMesh->addBindable(meshData.modelMatrix);

				if (skin) {
					auto jointMatrices = calculateJointMatrices(*skin, modelMatrix);
					std::size_t numJoints = std::min(jointMatrices.size(), static_cast<std::size_t>(kMaxJoints));
					meshData.jointMatrices = std::make_shared<graphics::UniformVariableValueVector<glm::mat4, kMaxJoints>>(
						"uJointMatrices", *it->program, jointMatrices.data(), numJoints
					);
					rMesh->addBindable(meshData.jointMatrices);
				}
			}
			else {
				SOMBRA_WARN_LOG << "RenderableMesh has a Pass " << pass << " not added to the CameraSystem";
			}
		}); });

		mGraphicsEngine.addRenderable(rMesh);
		SOMBRA_INFO_LOG << "Entity " << entity << " with RenderableMesh " << rMesh << " added successfully";
	}


	void RMeshSystem::onRemoveEntity(Entity entity)
	{
		auto [rMesh] = mEntityDatabase.getComponents<graphics::RenderableMesh>(entity);
		if (!rMesh) {
			SOMBRA_INFO_LOG << "Mesh Entity " << entity << " couldn't removed";
			return;
		}

		auto it = mRenderableMeshEntities.find(entity);
		if (it != mRenderableMeshEntities.end()) {
			mRenderableMeshEntities.erase(it);
		}

		mGraphicsEngine.removeRenderable(rMesh);
		SOMBRA_INFO_LOG << "Mesh Entity " << entity << " removed successfully";
	}


	void RMeshSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Meshes";

		mEntityDatabase.iterateComponents<TransformsComponent, Skin>(
			[&](Entity entity, TransformsComponent* transforms, Skin* skin) {
				if (transforms->updated.any()) {
					auto& meshData = mRenderableMeshEntities[entity];

					glm::mat4 translation	= glm::translate(glm::mat4(1.0f), transforms->position);
					glm::mat4 rotation		= glm::mat4_cast(transforms->orientation);
					glm::mat4 scale			= glm::scale(glm::mat4(1.0f), transforms->scale);
					glm::mat4 modelMatrix	= translation * rotation * scale;

					meshData.modelMatrix->setValue(modelMatrix);
					if (skin) {
						auto jointMatrices = calculateJointMatrices(*skin, modelMatrix);
						std::size_t numJoints = std::min(jointMatrices.size(), static_cast<std::size_t>(kMaxJoints));
						meshData.jointMatrices->setValue(jointMatrices.data(), numJoints);
					}
				}
			}
		);

		SOMBRA_INFO_LOG << "Update end";
	}

}
