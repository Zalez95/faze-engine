#ifndef LIGHT_SOURCE_SYSTEM
#define LIGHT_SOURCE_SYSTEM

#include <memory>
#include "ISystem.h"
#include "../graphics/core/UniformBuffer.h"
#include "../graphics/core/UniformVariable.h"

namespace se::app {

	/**
	 * Class LightSourceSystem, it's a System used for updating the
	 * Entities' light sources data
	 */
	class LightSourceSystem : public ISystem
	{
	private:	// Nested types
		struct ShaderLightSource;
		using UIntUniform = graphics::UniformVariableValue<unsigned int>;

	private:	// Attributes
		/** The maximum number of lights in the program */
		static constexpr unsigned int kMaxLights = 32;

		/** The UniformBuffer where the lights data will be stored */
		std::shared_ptr<graphics::UniformBuffer> mLightsBuffer;

	public:		// Functions
		/** Creates a new LightSourceSystem
		 *
		 * @param	entityDatabase the EntityDatabase that holds all the
		 *			Entities */
		LightSourceSystem(EntityDatabase& entityDatabase);

		/** Updates the light sources with the Entities
		 *
		 * @note	this function must be called from the thread with the
		 *			Graphics API context (probably thread 0) */
		virtual void update() override;
	};

}

#endif		// LIGHT_SOURCE_SYSTEM
