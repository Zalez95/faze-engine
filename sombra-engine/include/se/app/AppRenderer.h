#ifndef APP_RENDERER_H
#define APP_RENDERER_H

#include "../graphics/GraphicsEngine.h"
#include "../graphics/core/Texture.h"
#include "../graphics/core/UniformBuffer.h"

namespace se::app {

	/**
	 * Class AppRenderer, TODO:
	 */
	class AppRenderer
	{
	private:	// Nested types
		using TextureSPtr = std::shared_ptr<graphics::Texture>;
		using UniformBufferSPtr = std::shared_ptr<graphics::UniformBuffer>;

	private:	// Attributes
		/** The GraphicsEngine used for rendering the Scene */
		graphics::GraphicsEngine& mGraphicsEngine;

	public:		// Functions
		/** Creates a new AppRenderer
		 *
		 * @param	graphicsEngine the GraphicsEngine used for rendering the
		 *			Scene
		 * @param	width the initial width of the FrameBuffer where the
		 *			Scene is going to be rendered
		 * @param	height the initial height of the FrameBuffer where the
		 *			Scene is going to be rendered */
		AppRenderer(
			graphics::GraphicsEngine& graphicsEngine,
			std::size_t width, std::size_t height
		);

		/** Sets the number of lights
		 *
		 * @param	numLights the new number of lights */
		void setNumLights(unsigned int numLights);

		/** Sets the UniformBuffer where the lights data will be stored
		 *
		 * @param	lightsBuffer the new UniformBuffer with the lights data */
		void setLightsBuffer(UniformBufferSPtr lightsBuffer);

		/** Sets the irradiance texture of the AppRenderer
		 *
		 * @param	texture	the new irradiance texture */
		void setIrradianceMap(TextureSPtr texture);

		/** Sets the prefiltered environment map texture of the AppRenderer
		 *
		 * @param	texture	the new prefilter texture */
		void setPrefilterMap(TextureSPtr texture);

		/** Sets the convoluted BRDF texture of the AppRenderer
		 *
		 * @param	texture	the new BRDF texture */
		void setBRDFMap(TextureSPtr texture);

		/** Renders the graphics data of the Entities
		 *
		 * @note	this function must be called from the thread with the
		 *			Graphics API context (probably thread 0) */
		void render();
	};

}

#endif		// APP_RENDERER_H
