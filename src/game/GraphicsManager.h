#ifndef GRAPHICS_MANAGER_H
#define GRAPHICS_MANAGER_H

#include <map>
#include "../graphics/GraphicsSystem.h"
#include "../graphics/3D/Camera.h"
#include "../graphics/3D/Lights.h"
#include "../graphics/3D/Renderable3D.h"

namespace game {

	struct Entity;


	/**
	 * Class GraphicsManager, it's an Manager used for storing, updating and
	 * rendering the Entities' graphics data
	 */
	class GraphicsManager
	{
	private:	// Nested types
		typedef std::unique_ptr<graphics::Camera> CameraUPtr;
		typedef std::unique_ptr<graphics::PointLight> PointLightUPtr;
		typedef std::unique_ptr<graphics::Renderable3D> Renderable3DUPtr;

	private:	// Attributes
		/** The System used for rendering the data of the Entities */
		graphics::GraphicsSystem mGraphicsSystem;

		std::map<Entity*, CameraUPtr> mCameraEntities;
		std::map<Entity*, Renderable3DUPtr> mRenderable3DEntities;
		std::map<Entity*, PointLightUPtr> mPointLightEntities;

	public:		// Functions
		/** Creates a new GraphicsManager */
		GraphicsManager() {};

		/** Class destructor */
		~GraphicsManager() {};

		/** Adds the given Entity and its Camera data to the GraphicsManager
		 * 
		 * @param	entity a pointer to the Entity to add to the
		 *			GraphicsManager
		 * @param	Camera a pointer to the camera to add to the
		 *			GraphicsManager */
		void addEntity(Entity* entity, CameraUPtr camera);

		/** Adds the given Entity and its Renderable3D data to the
		 * GraphicsManager
		 * 
		 * @param	entity a pointer to the Entity to add to the
		 *			GraphicsManager
		 * @param	renderable3D a pointer to the Renderable3D to add to the
		 *			GraphicsManager */
		void addEntity(Entity* entity, Renderable3DUPtr renderable3D);

		/** Adds the given Entity and its PointLight data to the
		 * GraphicsManager
		 * 
		 * @param	entity a pointer to the Entity to add to the
		 *			GraphicsManager
		 * @param	pointLight a pointer to the PointLight to add to the
		 *			GraphicsManager */
		void addEntity(Entity* entity, PointLightUPtr pointLight);

		/** Removes the given Entity from the GraphicsManager so it won't
		 * longer be updated
		 * 
		 * @param	entity a pointer to the Entity to remove from the
		 *			GraphicsManager */
		void removeEntity(Entity* entity);

		/** Updates the graphics data with the Entities */
		void update();

		/** Renders the graphics data of the Entities */
		void render();
	};

}

#endif		// GRAPHICS_MANAGER_H
