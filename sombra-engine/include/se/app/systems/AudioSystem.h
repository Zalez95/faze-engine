#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include "../../audio/AudioEngine.h"
#include "EntitySystem.h"

namespace se::app {

	/**
	 * Class AudioSystem, it's an AppSystem used for updating the AudioEngine
	 * listener
	 */
	class AudioSystem : public EntitySystem
	{
	private:	// Attributes
		/** The AudioEngine used for playing the audio sources of the
		 * Entities */
		audio::AudioEngine& mAudioEngine;

	public:		// Functions
		/** Creates a new AudioSystem
		 *
		 * @param	appComponentDB a reference to the AppComponentDB that holds
		 *			all the Component of the Entities
		 * @param	audioEngine a reference to the AudioEngine used by
		 * 			the AudioSystem to play the audio sources */
		AudioSystem(
			AppComponentDB& appComponentDB, audio::AudioEngine& audioEngine
		);
	protected:
		/** Updates the sources data with the Entities location */
		virtual void execute() override;
	};

}

#endif		// AUDIO_SYSTEM_H
