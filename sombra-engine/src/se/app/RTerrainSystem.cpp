#include "se/utils/Log.h"
#include "se/graphics/Technique.h"
#include "se/graphics/Pass.h"
#include "se/app/RTerrainSystem.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"
#include "se/app/graphics/Camera.h"

namespace se::app {

	RTerrainSystem::RTerrainSystem(
		EntityDatabase& entityDatabase, graphics::GraphicsEngine& graphicsEngine, CameraSystem& cameraSystem
	) : ISystem(entityDatabase), mGraphicsEngine(graphicsEngine), mCameraSystem(cameraSystem)
	{
		mEntities.reserve(mEntityDatabase.getMaxEntities());
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<graphics::RenderableTerrain>());
	}


	RTerrainSystem::~RTerrainSystem()
	{
		mEntityDatabase.removeSystem(this);
	}


	void RTerrainSystem::onNewEntity(Entity entity)
	{
		auto [transforms, rTerrain] = mEntityDatabase.getComponents<TransformsComponent, graphics::RenderableTerrain>(entity);
		if (!rTerrain) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Terrain";
			return;
		}

		glm::mat4 modelMatrix(1.0f);
		if (transforms) {
			glm::mat4 translation	= glm::translate(glm::mat4(1.0f), transforms->position);
			glm::mat4 rotation		= glm::mat4_cast(transforms->orientation);
			glm::mat4 scale			= glm::scale(glm::mat4(1.0f), transforms->scale);
			modelMatrix = translation * rotation * scale;
		}

		auto& meshData = mRenderableTerrainEntities[entity];
		rTerrain->processTechniques([&, rTerrain = rTerrain](auto technique) { technique->processPasses([&](auto pass) {
			auto it = std::find_if(
				mCameraSystem.mPassesData.begin(), mCameraSystem.mPassesData.end(),
				[&](const auto& passData) { return passData.pass == pass; }
			);
			if (it != mCameraSystem.mPassesData.end()) {
				meshData.modelMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *it->program, modelMatrix);
				rTerrain->addBindable(meshData.modelMatrix);
			}
			else {
				SOMBRA_WARN_LOG << "RenderableTerrain has a Pass " << pass << " not added to the CameraSystem";
			}
		}); });

		if (mCameraSystem.getActiveCamera()) {
			rTerrain->setHighestLodLocation(mCameraSystem.getActiveCamera()->getPosition());
		}

		mGraphicsEngine.addRenderable(rTerrain);
		mEntities.push_back(entity);
		SOMBRA_INFO_LOG << "Entity " << entity << " with RenderableTerrain " << rTerrain << " added successfully";
	}


	void RTerrainSystem::onRemoveEntity(Entity entity)
	{
		auto [rTerrain] = mEntityDatabase.getComponents<graphics::RenderableTerrain>(entity);
		if (!rTerrain) {
			SOMBRA_INFO_LOG << "Terrain Entity " << entity << " couldn't removed";
			return;
		}

		auto itRT = mRenderableTerrainEntities.find(entity);
		if (itRT != mRenderableTerrainEntities.end()) {
			mRenderableTerrainEntities.erase(itRT);
		}

		mGraphicsEngine.removeRenderable(rTerrain);
		auto it = std::find(mEntities.begin(), mEntities.end(), entity);
		std::swap(*it, mEntities.back());
		mEntities.pop_back();
		SOMBRA_INFO_LOG << "Terrain Entity " << entity << " removed successfully";
	}


	void RTerrainSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Terrains";

		for (auto entity : mEntities) {
			auto [transforms, rTerrain] = mEntityDatabase.getComponents<TransformsComponent, graphics::RenderableTerrain>(entity);
			if (transforms && transforms->updated.any()) {
				auto& terrainData = mRenderableTerrainEntities[entity];

				glm::mat4 translation	= glm::translate(glm::mat4(1.0f), transforms->position);
				glm::mat4 rotation		= glm::mat4_cast(transforms->orientation);
				glm::mat4 scale			= glm::scale(glm::mat4(1.0f), transforms->scale);
				glm::mat4 modelMatrix	= translation * rotation * scale;

				terrainData.modelMatrix->setValue(modelMatrix);
			}

			if (mCameraSystem.getActiveCamera() && mCameraSystem.wasCameraUpdated()) {
				rTerrain->setHighestLodLocation(mCameraSystem.getActiveCamera()->getPosition());
			}
		}

		SOMBRA_INFO_LOG << "Update end";
	}

}
