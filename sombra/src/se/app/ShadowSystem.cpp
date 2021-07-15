#include "se/utils/Log.h"
#include "se/graphics/3D/Renderer3D.h"
#include "se/graphics/3D/FrustumFilter.h"
#include "se/graphics/RenderGraph.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/ViewportResolutionNode.h"
#include "se/app/ShadowSystem.h"
#include "se/app/Application.h"
#include "se/app/TransformsComponent.h"
#include "se/app/MeshComponent.h"
#include "se/app/TerrainComponent.h"
#include "se/app/CameraComponent.h"
#include "graphics/ShadowRenderSubGraph.h"
#include "graphics/DeferredLightRenderer.h"

namespace se::app {

	ShadowSystem::ShadowSystem(Application& application, const ShadowData& shadowData) :
		ISystem(application.getEntityDatabase()), mApplication(application), mShadowEntity(kNullEntity), mShadowData(shadowData)
	{
		mApplication.getEventManager()
			.subscribe(this, Topic::Shadow)
			.subscribe(this, Topic::RMesh)
			.subscribe(this, Topic::RShader)
			.subscribe(this, Topic::Shader);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask()
			.set<LightComponent>()
			.set<MeshComponent>()
			.set<TerrainComponent>()
		);

		auto& renderGraph = mApplication.getExternalTools().graphicsEngine->getRenderGraph();
		mShadowRenderSubGraph = dynamic_cast<ShadowRenderSubGraph*>(renderGraph.getNode("shadowRenderSubGraph"));
		mDeferredLightRenderer = dynamic_cast<DeferredLightRenderer*>(renderGraph.getNode("deferredLightRenderer"));

		mShadowRenderSubGraph->addShadow(mShadowData.resolution, glm::mat4(1.0f), glm::mat4(1.0f));
	}


	ShadowSystem::~ShadowSystem()
	{
		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager()
			.unsubscribe(this, Topic::Shader)
			.unsubscribe(this, Topic::RShader)
			.unsubscribe(this, Topic::RMesh)
			.unsubscribe(this, Topic::Shadow);
	}


	bool ShadowSystem::notify(const IEvent& event)
	{
		return tryCall(&ShadowSystem::onShadowEvent, event)
			|| tryCall(&ShadowSystem::onRMeshEvent, event)
			|| tryCall(&ShadowSystem::onRenderableShaderEvent, event)
			|| tryCall(&ShadowSystem::onShaderEvent, event);
	}


	void ShadowSystem::onNewComponent(Entity entity, const EntityDatabase::ComponentMask& mask)
	{
		tryCallC(&ShadowSystem::onNewLight, entity, mask);
		tryCallC(&ShadowSystem::onNewMesh, entity, mask);
		tryCallC(&ShadowSystem::onNewTerrain, entity, mask);
	}


	void ShadowSystem::onRemoveComponent(Entity entity, const EntityDatabase::ComponentMask& mask)
	{
		tryCallC(&ShadowSystem::onRemoveLight, entity, mask);
		tryCallC(&ShadowSystem::onRemoveMesh, entity, mask);
		tryCallC(&ShadowSystem::onRemoveTerrain, entity, mask);
	}


	void ShadowSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Renderers";

		auto [transforms, light] = mEntityDatabase.getComponents<TransformsComponent, LightComponent>(mShadowEntity, true);
		if (transforms && light) {
			CameraComponent camera;
			camera.setPosition(transforms->position);
			camera.setOrientation(transforms->orientation);
			if (light->source->type == LightSource::Type::Directional) {
				camera.setOrthographicProjection(
					-mShadowData.size, mShadowData.size, -mShadowData.size, mShadowData.size,
					mShadowData.zNear, mShadowData.zFar
				);
			}
			else if (light->source->type == LightSource::Type::Spot) {
				camera.setPerspectiveProjection(
					glm::radians(45.0f), 1.0f,
					mShadowData.zNear, mShadowData.zFar
				);
			}
			mShadowRenderSubGraph->setShadowVPMatrix(0, camera.getViewMatrix(), camera.getProjectionMatrix());

			transforms->updated.set(static_cast<int>(TransformsComponent::Update::Shadow));
		}

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	void ShadowSystem::onNewLight(Entity entity, LightComponent* light)
	{
		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " added successfully";
	}


	void ShadowSystem::onRemoveLight(Entity entity, LightComponent* light)
	{
		if (mShadowEntity == entity) {
			mShadowEntity = kNullEntity;
			SOMBRA_INFO_LOG << "Active Shadow Camera removed";
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " removed successfully";
	}


	void ShadowSystem::onNewMesh(Entity entity, MeshComponent* mesh)
	{
		mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
			mShadowRenderSubGraph->getShadowUniformsUpdater()->addRenderable(mesh->get(i));
			mesh->processRenderableShaders(i, [&](const auto& shader) {
				mShadowRenderSubGraph->getShadowUniformsUpdater()->addRenderableTechnique(mesh->get(i), shader->getTechnique());
			});
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with MeshComponent " << mesh << " added successfully";
	}


	void ShadowSystem::onRemoveMesh(Entity entity, MeshComponent* mesh)
	{
		mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
			mShadowRenderSubGraph->getShadowUniformsUpdater()->removeRenderable(mesh->get(i));
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with MeshComponent " << mesh << " removed successfully";
	}


	void ShadowSystem::onNewTerrain(Entity entity, TerrainComponent* terrain)
	{
		mShadowRenderSubGraph->getShadowUniformsUpdater()->addRenderable(terrain->get());
		terrain->processRenderableShaders([&](const auto& shader) {
			mShadowRenderSubGraph->getShadowUniformsUpdater()->addRenderableTechnique(terrain->get(), shader->getTechnique());
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with TerrainComponent " << terrain << " added successfully";
	}


	void ShadowSystem::onRemoveTerrain(Entity entity, TerrainComponent* terrain)
	{
		mShadowRenderSubGraph->getShadowUniformsUpdater()->removeRenderable(terrain->get());
		SOMBRA_INFO_LOG << "Entity " << entity << " with TerrainComponent " << terrain << " removed successfully";
	}


	void ShadowSystem::onShadowEvent(const ContainerEvent<Topic::Shadow, Entity>& event)
	{
		auto [transforms, light] = mEntityDatabase.getComponents<TransformsComponent, LightComponent>(event.getValue(), true);
		if (transforms && light && light->source) {
			mShadowEntity = event.getValue();

			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::Shadow));

			CameraComponent camera;
			camera.setPosition(transforms->position);
			camera.setOrientation(transforms->orientation);
			if (light->source->type == LightSource::Type::Directional) {
				camera.setOrthographicProjection(
					-mShadowData.size, mShadowData.size, -mShadowData.size, mShadowData.size,
					mShadowData.zNear, mShadowData.zFar
				);
			}
			else if (light->source->type == LightSource::Type::Spot) {
				camera.setPerspectiveProjection(
					glm::radians(45.0f), 1.0f,
					mShadowData.zNear, mShadowData.zFar
				);
			}
			mShadowRenderSubGraph->setShadowVPMatrix(0, camera.getViewMatrix(), camera.getProjectionMatrix());
		}
		else {
			SOMBRA_WARN_LOG << "Couldn't set Entity " << event.getValue() << " as Shadow Entity";
		}
	}


	void ShadowSystem::onRMeshEvent(const RMeshEvent& event)
	{
		auto [mesh] = mEntityDatabase.getComponents<MeshComponent>(event.getEntity(), true);
		if (mesh) {
			switch (event.getOperation()) {
				case RMeshEvent::Operation::Add:
					mShadowRenderSubGraph->getShadowUniformsUpdater()->addRenderable(mesh->get(event.getRIndex()));
					break;
				case RMeshEvent::Operation::Remove:
					mShadowRenderSubGraph->getShadowUniformsUpdater()->removeRenderable(mesh->get(event.getRIndex()));
					break;
			}
		}
	}


	void ShadowSystem::onRenderableShaderEvent(const RenderableShaderEvent& event)
	{
		if (event.getRComponentType() == RenderableShaderEvent::RComponentType::Mesh) {
			auto [mesh] = mEntityDatabase.getComponents<MeshComponent>(event.getEntity(), true);
			if (mesh) {
				switch (event.getOperation()) {
					case RenderableShaderEvent::Operation::Add:
						mShadowRenderSubGraph->getShadowUniformsUpdater()->addRenderableTechnique(mesh->get(event.getRIndex()), event.getShader()->getTechnique());
						break;
					case RenderableShaderEvent::Operation::Remove:
						mShadowRenderSubGraph->getShadowUniformsUpdater()->removeRenderableTechnique(mesh->get(event.getRIndex()), event.getShader()->getTechnique());
						break;
				}
			}
		}
		else {
			graphics::Renderable* renderable = nullptr;
			if (event.getRComponentType() == RenderableShaderEvent::RComponentType::Terrain) {
				auto [terrain] = mEntityDatabase.getComponents<TerrainComponent>(event.getEntity(), true);
				renderable = &terrain->get();
			}

			if (renderable) {
				switch (event.getOperation()) {
					case RenderableShaderEvent::Operation::Add:
						mShadowRenderSubGraph->getShadowUniformsUpdater()->addRenderableTechnique(*renderable, event.getShader()->getTechnique());
						break;
					case RenderableShaderEvent::Operation::Remove:
						mShadowRenderSubGraph->getShadowUniformsUpdater()->removeRenderableTechnique(*renderable, event.getShader()->getTechnique());
						break;
				}
			}
		}
	}


	void ShadowSystem::onShaderEvent(const ShaderEvent& event)
	{
		switch (event.getOperation()) {
			case ShaderEvent::Operation::Add:
				mShadowRenderSubGraph->getShadowUniformsUpdater()->onAddTechniquePass(event.getShader()->getTechnique(), event.getStep()->getPass());
				break;
			case ShaderEvent::Operation::Remove:
				mShadowRenderSubGraph->getShadowUniformsUpdater()->onRemoveTechniquePass(event.getShader()->getTechnique(), event.getStep()->getPass());
				break;
		}
	}

}
