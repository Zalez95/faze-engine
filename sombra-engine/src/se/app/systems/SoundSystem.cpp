#include "se/utils/Log.h"
#include "se/app/systems/SoundSystem.h"

namespace se::app {

	SoundSystem::SoundSystem(AppComponentDB& appComponentDB) : EntitySystem(appComponentDB)
	{
		setAccessPolicy<ComponentId::Transforms>(AccessPolicy::Read);
		setAccessPolicy<ComponentId::AudioSource>(AccessPolicy::Write);
	}


	void SoundSystem::execute()
	{
		SOMBRA_INFO_LOG << "Updating the SoundSystem";

		executeForEach([this](AppComponentDB::EntityId entity) {
			const auto& transforms = getComponentW<ComponentId::Transforms>(entity);
			auto& source = getComponentW<ComponentId::AudioSource>(entity);

			if (transforms.updated.any()) {
				source->setPosition(transforms.position);
				source->setOrientation(
					glm::vec3(0.0f, 0.0f, 1.0f) * transforms.orientation,
					glm::vec3(0.0f, 1.0f, 0.0)
				);
				source->setVelocity(transforms.velocity);
			}
		});

		SOMBRA_INFO_LOG << "SoundSystem updated";
	}

}
