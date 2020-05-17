#ifndef SOUND_SYSTEM_H
#define SOUND_SYSTEM_H

#include "EntitySystem.h"

namespace se::app {

	/**
	 * Class SoundSystem, it's an AppSystem used for updating and playing the
	 * Entities' audio sources
	 */
	class SoundSystem : public EntitySystem
	{
	public:		// Functions
		/** Creates a new SoundSystem
		 *
		 * @param	appComponentDB a reference to the AppComponentDB that holds
		 *			all the Component of the Entities */
		SoundSystem(AppComponentDB& appComponentDB);
	protected:
		/** Updates the sources data with the Entities location */
		virtual void execute() override;
	};

}

#endif		// SOUND_SYSTEM_H
