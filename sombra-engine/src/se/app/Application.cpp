#include <chrono>
#include <thread>
#include <iostream>
#include "se/utils/Log.h"
#include "se/window/WindowSystem.h"
#include "se/graphics/GraphicsSystem.h"
#include "se/physics/PhysicsEngine.h"
#include "se/animation/AnimationSystem.h"
#include "se/audio/AudioEngine.h"
#include "se/app/Entity.h"
#include "se/app/Application.h"
#include "se/app/InputManager.h"
#include "se/app/GraphicsManager.h"
#include "se/app/PhysicsManager.h"
#include "se/app/CollisionManager.h"
#include "se/app/AnimationManager.h"
#include "se/app/AudioManager.h"

namespace se::app {

	Application::Application(const std::string& title, int width, int height, float updateTime) :
		mUpdateTime(updateTime), mState(AppState::Stopped)
	{
		SOMBRA_INFO_LOG << "Creating the Application";
		try {
			// Window
			mWindowSystem = new window::WindowSystem({ title, width, height, false, false });
			SOMBRA_INFO_LOG << mWindowSystem->getGLInfo();

			// Input
			mInputManager = new InputManager(*mWindowSystem);

			// Graphics
			mGraphicsSystem = new graphics::GraphicsSystem();
			mGraphicsManager = new GraphicsManager(*mGraphicsSystem);

			// Physics
			mPhysicsEngine = new physics::PhysicsEngine();
			mPhysicsManager = new PhysicsManager(*mPhysicsEngine);

			// Collision
			mCollisionDetector = new collision::CollisionDetector();
			mCollisionManager = new CollisionManager(*mCollisionDetector, *mPhysicsEngine);

			// Animation
			mAnimationSystem = new animation::AnimationSystem();
			mAnimationManager = new AnimationManager(*mAnimationSystem);

			// Audio
			mAudioEngine = new audio::AudioEngine();
			mAudioManager = new AudioManager(*mAudioEngine);
		}
		catch (std::exception& e) {
			mState = AppState::Error;
			SOMBRA_ERROR_LOG << " Error creating the application: " << e.what();
		}
		SOMBRA_INFO_LOG << "Application created";
	}


	Application::~Application()
	{
		SOMBRA_INFO_LOG << "Deleting the Application";
		delete mAudioManager;
		delete mAudioEngine;
		delete mAnimationManager;
		delete mAnimationSystem;
		delete mCollisionManager;
		delete mCollisionDetector;
		delete mPhysicsManager;
		delete mPhysicsEngine;
		delete mGraphicsManager;
		delete mGraphicsSystem;
		delete mInputManager;
		delete mWindowSystem;
		SOMBRA_INFO_LOG << "Application deleted";
	}


	bool Application::run()
	{
		SOMBRA_INFO_LOG << "Start running";
		init();

		if (mState != AppState::Stopped) {
			SOMBRA_ERROR_LOG << " Bad game state";
			return false;
		}

		/*********************************************************************
		 * MAIN LOOP
		 *********************************************************************/
		mState = AppState::Running;
		float lastTime = mWindowSystem->getTime();
		while (mState == AppState::Running) {
			// Calculate the elapsed time since the last update
			float curTime	= mWindowSystem->getTime();
			float deltaTime	= curTime - lastTime;

			if (deltaTime >= mUpdateTime) {
				lastTime = curTime;
				std::cout << deltaTime << "ms\r";

				// Update the Systems
				SOMBRA_INFO_LOG << "Update phase";
				mWindowSystem->update();
				if (mWindowSystem->getInputData()->keys[SE_KEY_ESCAPE]) {
					mState = AppState::Stopped;
				}
				mInputManager->update();
				mPhysicsManager->doDynamics(deltaTime);
				mCollisionManager->update(deltaTime);
				mPhysicsManager->doConstraints(deltaTime);
				mAnimationManager->update(deltaTime);
				mAudioManager->update();
				mGraphicsManager->update();

				// Draw
				SOMBRA_INFO_LOG << "Render phase";
				mGraphicsManager->render();
				mWindowSystem->swapBuffers();
			}
			else {
				SOMBRA_DEBUG_LOG << "Wait " << mUpdateTime - deltaTime << " seconds";
				std::this_thread::sleep_for( std::chrono::duration<float>(mUpdateTime - deltaTime) );
			}
		}

		end();
		SOMBRA_INFO_LOG << "End running";
		return true;
	}

}
