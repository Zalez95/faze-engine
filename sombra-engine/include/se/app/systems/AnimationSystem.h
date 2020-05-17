#ifndef ANIMATION_SYSTEM_H
#define ANIMATION_SYSTEM_H

#include "../../animation/AnimationEngine.h"
#include "EntitySystem.h"

namespace se::app {

	/**
	 * Class AnimationSystem, it's an AppSystem used for updating the
	 * Entities' AnimationNodes
	 */
	class AnimationSystem : public EntitySystem
	{
	private:	// Attributes
		/** The AnimationEngine used for playing the animations of the
		 * Entities */
		animation::AnimationEngine& mAnimationEngine;

	public:		// Functions
		/** Creates a new AnimationSystem
		 *
		 * @param	appComponentDB a reference to the AppComponentDB that holds
		 *			all the Component of the Entities
		 * @param	animationEngine a reference to the AnimationEngine used for
		 *			playing the animations of the Entities */
		AnimationSystem(
			AppComponentDB& appComponentDB,
			animation::AnimationEngine& animationEngine
		);
	protected:
		/** Updates the AnimationNodes with the Entities location */
		virtual void execute() override;
	};

}

#endif		// ANIMATION_SYSTEM_H
