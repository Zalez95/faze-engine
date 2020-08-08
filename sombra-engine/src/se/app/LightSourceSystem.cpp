#include <array>
#include "se/utils/Log.h"
#include "se/utils/FixedVector.h"
#include "se/app/LightSourceSystem.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"
#include "se/app/graphics/LightSource.h"

namespace se::app {

	struct LightSourceSystem::ShaderLightSource
	{
		glm::vec3 position;
		float padding[1];
		glm::vec3 direction;
		unsigned int type;
		glm::vec4 color;
		float intensity;
		float range;
		float lightAngleScale;
		float lightAngleOffset;
	};


	LightSourceSystem::LightSourceSystem(EntityDatabase& entityDatabase) :
		ISystem(entityDatabase)
	{
		// FIXME: mNumLights
		mLightsBuffer = std::make_shared<graphics::UniformBuffer>();
		utils::FixedVector<ShaderLightSource, kMaxLights> lightsBufferData(kMaxLights);
		mLightsBuffer->resizeAndCopy(lightsBufferData.data(), lightsBufferData.size());
	}


	void LightSourceSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the LightSourceSystems";

		unsigned int i = 0;
		std::array<ShaderLightSource, kMaxLights> uBaseLights;
		mEntityDatabase.iterateComponents<TransformsComponent, LightSource>(
			[&](Entity, TransformsComponent* transforms, LightSource* light) {
				if (i < kMaxLights) {
					uBaseLights[i].type = static_cast<unsigned int>(light->type);
					uBaseLights[i].position = transforms->position;
					uBaseLights[i].direction = glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
					uBaseLights[i].color = { light->color, 1.0f };
					uBaseLights[i].intensity = light->intensity;
					switch (light->type) {
						case LightSource::Type::Directional: {
							uBaseLights[i].range = std::numeric_limits<float>::max();
						} break;
						case LightSource::Type::Point: {
							uBaseLights[i].range = light->range;
						} break;
						case LightSource::Type::Spot: {
							uBaseLights[i].range = light->range;
							uBaseLights[i].lightAngleScale = 1.0f / std::max(0.001f, std::cos(light->innerConeAngle) - std::cos(light->outerConeAngle));
							uBaseLights[i].lightAngleOffset = -std::cos(light->outerConeAngle) * uBaseLights[i].lightAngleScale;
						} break;
					}
					++i;
				}
			}
		);

		//TODO:mNumLights->setValue(i);
		mLightsBuffer->copy(uBaseLights.data(), i);

		SOMBRA_INFO_LOG << "Update end";
	}

}
