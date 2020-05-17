#include "se/utils/Log.h"
#include "se/app/systems/AudioSystem.h"

namespace se::app {

	AudioSystem::AudioSystem(AppComponentDB& appComponentDB, audio::AudioEngine& audioEngine) :
		EntitySystem(appComponentDB), mAudioEngine(audioEngine)
	{
		setAccessPolicy<ComponentId::Transforms>(AccessPolicy::Read);
	}


	void AudioSystem::execute()
	{
		SOMBRA_INFO_LOG << "Updating the AudioSystem";

		executeForEach([this](AppComponentDB::EntityId entity) {
			const auto& transforms = getComponentR<ComponentId::Transforms>(entity);

			mAudioEngine.setListenerPosition(transforms.position);
			mAudioEngine.setListenerOrientation(
				glm::vec3(0.0f, 0.0f, 1.0f) * transforms.orientation,
				glm::vec3(0.0f, 1.0f, 0.0)
			);
			mAudioEngine.setListenerVelocity(transforms.velocity);
		});

		SOMBRA_INFO_LOG << "AudioSystem updated";
	}

}
