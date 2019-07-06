#include <glm/gtc/matrix_transform.hpp>
#include "se/app/GraphicsManager.h"
#include "se/app/Entity.h"
#include "se/app/events/ResizeEvent.h"
#include "se/app/RawMesh.h"
#include "se/app/events/CollisionEvent.h"
#include "se/app/loaders/MeshLoader.h"
#include "se/collision/Manifold.h"
#include "se/graphics/3D/Mesh.h"
#include "se/graphics/3D/Material.h"
#include "se/graphics/3D/Renderable3D.h"
#include "se/utils/Log.h"

namespace se::app {

	GraphicsManager::GraphicsManager(graphics::GraphicsSystem& graphicsSystem, EventManager& eventManager) :
		mGraphicsSystem(graphicsSystem), mEventManager(eventManager)
	{
		RawMesh rawMesh1("Cube");
		rawMesh1.positions = {
			{ 0.5f, 0.5f,-0.5f},
			{ 0.5f,-0.5f,-0.5f},
			{-0.5f,-0.5f,-0.5f},
			{-0.5f, 0.5f,-0.5f},
			{ 0.5f, 0.5f, 0.5f},
			{ 0.5f,-0.5f, 0.5f},
			{-0.5f,-0.5f, 0.5f},
			{-0.5f, 0.5f, 0.5f},
			{ 0.5f, 0.5f,-0.5f},
			{ 0.5f,-0.5f,-0.5f},
			{ 0.5f, 0.5f, 0.5f},
			{ 0.5f,-0.5f, 0.5f},
			{ 0.5f, 0.5f, 0.5f},
			{ 0.5f,-0.5f, 0.5f},
			{-0.5f,-0.5f, 0.5f},
			{-0.5f, 0.5f, 0.5f},
			{ 0.5f, 0.5f,-0.5f},
			{ 0.5f,-0.5f,-0.5f},
			{-0.5f, 0.5f,-0.5f},
			{-0.5f, 0.5f,-0.5f},
			{-0.5f,-0.5f,-0.5f},
			{-0.5f,-0.5f,-0.5f},
			{-0.5f,-0.5f, 0.5f},
			{-0.5f, 0.5f, 0.5f}
		};
		rawMesh1.texCoords = {
			{0.666467010f, 0.666466951f},
			{0.999800264f, 0.000199760f},
			{0.333533257f, 0.333133578f},
			{0.333533287f, 0.666466951f},
			{0.666467010f, 0.333533167f},
			{0.999800145f, 0.333133548f},
			{0.333533197f, 0.000199760f},
			{0.333533197f, 0.333533257f},
			{0.333133667f, 0.333533167f},
			{0.000199899f, 0.333533197f},
			{0.333133548f, 0.666466951f},
			{0.000199760f, 0.666466951f},
			{0.333133697f, 0.333133548f},
			{0.333133488f, 0.000199760f},
			{0.000199760f, 0.000199909f},
			{0.000199869f, 0.333133667f},
			{0.333133548f, 0.999800264f},
			{0.000199760f, 0.999800264f},
			{0.333133548f, 0.666866540f},
			{0.666467010f, 0.333133488f},
			{0.000199770f, 0.666866540f},
			{0.666866540f, 0.000199799f},
			{0.666866540f, 0.333133578f},
			{0.666466891f, 0.000199760f}
		};
		rawMesh1.faceIndices = {
			16, 20, 18,
			5, 21, 1,
			2, 23, 19,
			0, 7, 4,
			10, 9, 8,
			15, 13, 12,
			16, 17, 20,
			5, 22, 21,
			2, 6, 23,
			0, 3, 7,
			10, 11, 9,
			15, 14, 13
		};
		rawMesh1.normals = MeshLoader::calculateNormals(rawMesh1.positions, rawMesh1.faceIndices);
		rawMesh1.tangents = MeshLoader::calculateTangents(rawMesh1.positions, rawMesh1.texCoords, rawMesh1.faceIndices);
		mCubeMesh = std::make_shared<se::graphics::Mesh>(MeshLoader::createGraphicsMesh(rawMesh1));

		RawMesh rawMesh2("tetrahedron");
		rawMesh2.positions = {
			{ 0.0f, 0.5f, 0.0f },
			{ 0.433012723f, -0.25f, 0.0f },
			{ -0.433012723f, -0.25f, 0.0f },
			{ 0.0f, 0.0f, 1.0f }
		};
		rawMesh2.texCoords = {
			{ 0.0f, 1.0f },
			{ 0.0f, 0.0f },
			{ 0.0f, 1.0f },
			{ 0.0f, 1.0f }
		};
		rawMesh2.faceIndices = {
			0, 1, 2,
			3, 0, 1,
			3, 1, 2,
			3, 2, 0
		};
		rawMesh2.normals = MeshLoader::calculateNormals(rawMesh2.positions, rawMesh2.faceIndices);
		rawMesh2.tangents = MeshLoader::calculateTangents(rawMesh2.positions, rawMesh2.texCoords, rawMesh2.faceIndices);
		mTetrahedronMesh = std::make_shared<se::graphics::Mesh>(MeshLoader::createGraphicsMesh(rawMesh2));

		mYellowMaterial = std::shared_ptr<se::graphics::Material>(new se::graphics::Material{
			"tmp_material",
			se::graphics::PBRMetallicRoughness{ { 1.0f, 1.0f, 0.0f, 1.0f }, nullptr, 0.2f, 0.5f, nullptr },
			nullptr, 1.0f, nullptr, 1.0f, nullptr, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
		});
		mBlueMaterial = std::shared_ptr<se::graphics::Material>(new se::graphics::Material{
			"tmp_material",
			se::graphics::PBRMetallicRoughness{ { 0.0f, 0.0f, 1.0f, 1.0f }, nullptr, 0.2f, 0.5f, nullptr },
			nullptr, 1.0f, nullptr, 1.0f, nullptr, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
		});
		mRedMaterial = std::shared_ptr<se::graphics::Material>(new se::graphics::Material{
			"tmp_material",
			se::graphics::PBRMetallicRoughness{ { 1.0f, 0.0f, 0.0f, 1.0f }, nullptr, 0.2f, 0.5f, nullptr },
			nullptr, 1.0f, nullptr, 1.0f, nullptr, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
		});

		mGraphicsSystem.addLayer(&mLayer3D);
		mEventManager.subscribe(this, Topic::Resize);
		mEventManager.subscribe(this, Topic::Collision);
	}


	GraphicsManager::~GraphicsManager()
	{
		mEventManager.unsubscribe(this, Topic::Collision);
		mEventManager.unsubscribe(this, Topic::Resize);
	}


	void GraphicsManager::notify(const IEvent& event)
	{
		tryCall(&GraphicsManager::onResizeEvent, event);
		tryCall(&GraphicsManager::onCollisionEvent, event);
	}


	void GraphicsManager::addCameraEntity(Entity* entity, CameraUPtr camera)
	{
		if (!entity || !camera) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Camera";
			return;
		}

		// The Camera initial data is overridden by the entity one
		graphics::Camera* cPtr = camera.get();
		cPtr->setPosition(entity->position);
		cPtr->setTarget(entity->position + glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation);
		cPtr->setUp({ 0.0f, 1.0f, 0.0f });

		// Add the Camera
		mLayer3D.setCamera(cPtr);
		mCameraEntities.emplace(entity, std::move(camera));
		SOMBRA_INFO_LOG << "Entity " << entity << " with Camera " << cPtr << " added successfully";
	}


	void GraphicsManager::addRenderableEntity(Entity* entity, Renderable3DUPtr renderable3D, SkinSPtr skin)
	{
		if (!entity || !renderable3D) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Renderable3D";
			return;
		}

		// The Renderable3D initial data is overridden by the entity one
		graphics::Renderable3D* rPtr = renderable3D.get();
		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), entity->position);
		glm::mat4 rotation		= glm::mat4_cast(entity->orientation);
		glm::mat4 scale			= glm::scale(glm::mat4(1.0f), entity->scale);
		rPtr->setModelMatrix(translation * rotation * scale);
		if (skin) {
			rPtr->setJointMatrices( calculateJointMatrices(*skin, rPtr->getModelMatrix()) );
		}

		// Add the Renderable3D
		mLayer3D.addRenderable3D(rPtr);
		mRenderable3DEntities.emplace(entity, std::move(renderable3D));
		if (skin) {
			mRenderable3DSkins.emplace(rPtr, skin);
			SOMBRA_INFO_LOG << "Entity " << entity << " with Renderable3D " << rPtr << " and skin " << skin.get() << " added successfully";
		}
		else {
			SOMBRA_INFO_LOG << "Entity " << entity << " with Renderable3D " << rPtr << " added successfully";
		}
	}


	void GraphicsManager::addSkyEntity(Entity* entity, Renderable3DUPtr renderable3D)
	{
		if (!entity || !renderable3D) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Sky Renderable3D";
			return;
		}

		// The Renderable3D initial data is overridden by the entity one
		graphics::Renderable3D* rPtr = renderable3D.get();
		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), entity->position);
		glm::mat4 rotation		= glm::mat4_cast(entity->orientation);
		glm::mat4 scale			= glm::scale(glm::mat4(1.0f), entity->scale);
		rPtr->setModelMatrix(translation * rotation * scale);

		// Add the Renderable3D
		mLayer3D.setSky(rPtr);
		mSkyEntities.emplace(entity, std::move(renderable3D));
		SOMBRA_INFO_LOG << "Entity " << entity << " with Sky Renderable3D " << rPtr << " added successfully";
	}


	void GraphicsManager::addTerrainEntity(Entity* entity, RenderableTerrainUPtr renderable)
	{
		if (!entity || !renderable) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as RenderableTerrain";
			return;
		}

		// Add the Renderable3D
		graphics::RenderableTerrain* rPtr = renderable.get();
		mLayer3D.setTerrain(rPtr);
		mRenderableTerrainEntities.emplace(entity, std::move(renderable));
		SOMBRA_INFO_LOG << "Entity " << entity << " with RenderableTerrain " << rPtr << " added successfully";
	}


	void GraphicsManager::addLightEntity(Entity* entity, LightUPtr light)
	{
		if (!entity || !light) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as ILight";
			return;
		}

		// The PointLight initial data is overridden by the entity one
		graphics::ILight* lPtr = light.get();
		if (auto dLight = (dynamic_cast<graphics::DirectionalLight*>(lPtr))) {
			dLight->direction = glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation;
		}
		else if (auto pLight = (dynamic_cast<graphics::PointLight*>(lPtr))) {
			pLight->position = entity->position;
		}
		else if (auto sLight = (dynamic_cast<graphics::SpotLight*>(lPtr))) {
			sLight->position = entity->position;
			sLight->direction = glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation;
		}

		// Add the ILight
		mLayer3D.addLight(lPtr);
		mLightEntities.emplace(entity, std::move(light));
		SOMBRA_INFO_LOG << "Entity " << entity << " with ILight " << lPtr << " added successfully";
	}


	void GraphicsManager::removeEntity(Entity* entity)
	{
		auto itCamera = mCameraEntities.find(entity);
		if (itCamera != mCameraEntities.end()) {
			mLayer3D.setCamera(nullptr);
			mCameraEntities.erase(itCamera);
			SOMBRA_INFO_LOG << "Camera Entity " << entity << " removed successfully";
		}

		auto [itRenderable3DBegin, itRenderable3DEnd] = mRenderable3DEntities.equal_range(entity);
		for (auto itRenderable3D = itRenderable3DBegin; itRenderable3D != itRenderable3DEnd;) {
			mLayer3D.removeRenderable3D(itRenderable3D->second.get());
			itRenderable3D = mRenderable3DEntities.erase(itRenderable3D);
			SOMBRA_INFO_LOG << "Renderable3D Entity " << entity << " removed successfully";
		}

		auto itSky = mSkyEntities.find(entity);
		if (itSky != mSkyEntities.end()) {
			mLayer3D.setSky(nullptr);
			mSkyEntities.erase(itSky);
			SOMBRA_INFO_LOG << "Sky Renderable3D Entity " << entity << " removed successfully";
		}

		auto itRenderableTerrain = mRenderableTerrainEntities.find(entity);
		if (itRenderableTerrain != mRenderableTerrainEntities.end()) {
			mLayer3D.setTerrain(nullptr);
			mRenderableTerrainEntities.erase(itRenderableTerrain);
			SOMBRA_INFO_LOG << "RenderableTerrain Entity " << entity << " removed successfully";
		}

		auto itLight = mLightEntities.find(entity);
		if (itLight != mLightEntities.end()) {
			mLayer3D.removeLight(itLight->second.get());
			mLightEntities.erase(itLight);
			SOMBRA_INFO_LOG << "ILight Entity " << entity << " removed successfully";
		}
	}


	void GraphicsManager::update()
	{
		SOMBRA_INFO_LOG << "Update start";

		SOMBRA_DEBUG_LOG << "Updating the Cameras";
		bool activeCameraUpdated = false;
		for (auto& pair : mCameraEntities) {
			Entity* entity = pair.first;
			graphics::Camera* camera = pair.second.get();

			if (entity->updated.any()) {
				camera->setPosition(entity->position);
				camera->setTarget(entity->position + glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation);
				camera->setUp({ 0.0f, 1.0f, 0.0f });

				if (camera == mLayer3D.getCamera()) {
					activeCameraUpdated = true;
				}
			}
		}

		SOMBRA_DEBUG_LOG << "Updating the Renderable3Ds";
		for (auto& pair : mRenderable3DEntities) {
			Entity* entity = pair.first;
			graphics::Renderable3D* renderable3D = pair.second.get();

			if (entity->updated.any()) {
				glm::mat4 translation	= glm::translate(glm::mat4(1.0f), entity->position);
				glm::mat4 rotation		= glm::mat4_cast(entity->orientation);
				glm::mat4 scale			= glm::scale(glm::mat4(1.0f), entity->scale);
				renderable3D->setModelMatrix(translation * rotation * scale);
			}

			// Set the joint matrices of the skeletal animation
			auto itSkin = mRenderable3DSkins.find(renderable3D);
			if (itSkin != mRenderable3DSkins.end()) {
				const Skin& skin = *itSkin->second;
				renderable3D->setJointMatrices( calculateJointMatrices(skin, renderable3D->getModelMatrix()) );
			}
		}

		SOMBRA_DEBUG_LOG << "Updating the RenderableTerrains";
		for (auto& pair : mRenderableTerrainEntities) {
			graphics::RenderableTerrain* renderableTerrain = pair.second.get();

			if (activeCameraUpdated) {
				renderableTerrain->update(*mLayer3D.getCamera());
			}
		}

		SOMBRA_DEBUG_LOG << "Updating the ILights";
		for (auto& pair : mLightEntities) {
			Entity* entity = pair.first;
			graphics::ILight* light = pair.second.get();

			if (entity->updated.any()) {
				if (auto dLight = (dynamic_cast<graphics::DirectionalLight*>(light))) {
					dLight->direction = glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation;
				}
				else if (auto pLight = (dynamic_cast<graphics::PointLight*>(light))) {
					pLight->position = entity->position;
				}
				else if (auto sLight = (dynamic_cast<graphics::SpotLight*>(light))) {
					sLight->position = entity->position;
					sLight->direction = glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation;
				}
			}
		}

		SOMBRA_INFO_LOG << "Update end";
	}


	void GraphicsManager::render()
	{
		SOMBRA_INFO_LOG << "Render start";
		mGraphicsSystem.render();
		SOMBRA_INFO_LOG << "Render end";
	}

// Private functions
	void GraphicsManager::onResizeEvent(const ResizeEvent& event)
	{
		auto width = static_cast<unsigned int>(event.getWidth());
		auto height = static_cast<unsigned int>(event.getHeight());
		mGraphicsSystem.setViewport({ width, height });
	}


	void GraphicsManager::onCollisionEvent(const CollisionEvent& event)
	{
		SOMBRA_TRACE_LOG << "Received CollisionEvent: " << event;

		const collision::Manifold* manifold = event.getManifold();
		for (const se::collision::Contact& c : manifold->contacts) {
			// Yellow Cube = collider 0 contact point
			auto r3d1 = std::make_unique<se::graphics::Renderable3D>(mCubeMesh, mYellowMaterial);
			r3d1->setModelMatrix( glm::translate(glm::mat4(1.0f), c.worldPosition[0]) * glm::scale(glm::mat4(1.0f), glm::vec3(0.05f)) );
			mLayer3D.addRenderable3D(r3d1.get());
			mOtherRenderable3Ds.push_back(std::move(r3d1));

			// Red Cube = collider 1 contact point
			auto r3d2 = std::make_unique<se::graphics::Renderable3D>(mCubeMesh, mRedMaterial);
			r3d2->setModelMatrix(glm::translate(glm::mat4(1.0f), c.worldPosition[1]) * glm::scale(glm::mat4(1.0f), glm::vec3(0.05f)));
			mLayer3D.addRenderable3D(r3d2.get());
			mOtherRenderable3Ds.push_back(std::move(r3d2));

			// Blue tetrahedron = separation direction from collider 1 to collider 0
			glm::vec3 new_y = c.normal;
			glm::vec3 new_z = glm::normalize(glm::cross(c.normal, glm::vec3(0.0f, 1.0f, 0.0f)));
			glm::vec3 new_x = glm::normalize(glm::cross(new_y, new_z));
			glm::mat4 rotMatrix = glm::mat4(glm::mat3(new_x, new_y, new_z)) * glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));

			auto r3d3 = std::make_unique<se::graphics::Renderable3D>(mTetrahedronMesh, mBlueMaterial);
			r3d3->setModelMatrix(
				glm::translate(glm::mat4(1.0f), c.worldPosition[1])
				* rotMatrix
				* glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, c.penetration))
			);
			mLayer3D.addRenderable3D(r3d3.get());
			mOtherRenderable3Ds.push_back(std::move(r3d3));
		}
	}

}
