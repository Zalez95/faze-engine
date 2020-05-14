#include <thread>
#include "se/utils/Log.h"
#include "se/window/WindowSystem.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/physics/PhysicsEngine.h"
#include "se/animation/AnimationSystem.h"
#include "se/audio/AudioEngine.h"
#include "se/utils/TaskSet.h"
#include "se/app/Application.h"
#include "se/app/InputManager.h"
#include "se/app/GraphicsManager.h"
#include "se/app/PhysicsManager.h"
#include "se/app/CollisionManager.h"
#include "se/app/AnimationManager.h"
#include "se/app/AudioManager.h"
#include "se/app/events/EventManager.h"
#include "se/app/gui/GUIManager.h"

namespace se::app {

	Application::Application(
		const window::WindowData& windowConfig,
		const graphics::GraphicsData& graphicsConfig,
		const collision::CollisionWorldData& collisionConfig,
		float updateTime
	) : mUpdateTime(updateTime), mState(AppState::Stopped), mStopRunning(false),
		mWindowSystem(nullptr), mGraphicsEngine(nullptr), mPhysicsEngine(nullptr), mCollisionWorld(nullptr),
		mAnimationSystem(nullptr), mAudioEngine(nullptr), mTaskManager(nullptr),
		mEventManager(nullptr), mInputManager(nullptr), mGraphicsManager(nullptr), mPhysicsManager(nullptr),
		mCollisionManager(nullptr), mAnimationManager(nullptr), mAudioManager(nullptr)
	{
		SOMBRA_INFO_LOG << "Creating the Application";
		try {
			// Tasks
			mTaskManager = new utils::TaskManager(kMaxTasks);

			// Events
			mEventManager = new EventManager();

			// Window
			mWindowSystem = new window::WindowSystem(windowConfig);

			// Input
			mInputManager = new InputManager(*mWindowSystem, *mEventManager);

			// Graphics
			mGraphicsEngine = new graphics::GraphicsEngine(graphicsConfig);
			mGraphicsManager = new GraphicsManager(*mGraphicsEngine, *mEventManager);
			mGUIManager = new GUIManager(*mEventManager, *mGraphicsManager, { windowConfig.width, windowConfig.height });

			// Physics
			mPhysicsEngine = new physics::PhysicsEngine(kBaseBias);
			mPhysicsManager = new PhysicsManager(*mPhysicsEngine, *mEventManager);

			// Collision
			mCollisionWorld = new collision::CollisionWorld(collisionConfig);
			mCollisionManager = new CollisionManager(*mCollisionWorld, *mEventManager);

			// Animation
			mAnimationSystem = new animation::AnimationSystem();
			mAnimationManager = new AnimationManager(*mAnimationSystem);

			// Audio
			mAudioEngine = new audio::AudioEngine();
			mAudioManager = new AudioManager(*mAudioEngine);
		}
		catch (std::exception& e) {
			mState = AppState::Error;
			SOMBRA_ERROR_LOG << " Error while creating the Application: " << e.what();
		}
		SOMBRA_INFO_LOG << "Application created";
	}


	Application::~Application()
	{
		SOMBRA_INFO_LOG << "Deleting the Application";
		if (mGUIManager) { delete mGUIManager; }
		if (mAudioManager) { delete mAudioManager; }
		if (mAudioEngine) { delete mAudioEngine; }
		if (mAnimationManager) { delete mAnimationManager; }
		if (mAnimationSystem) { delete mAnimationSystem; }
		if (mCollisionManager) { delete mCollisionManager; }
		if (mCollisionWorld) { delete mCollisionWorld; }
		if (mPhysicsManager) { delete mPhysicsManager; }
		if (mPhysicsEngine) { delete mPhysicsEngine; }
		if (mGraphicsManager) { delete mGraphicsManager; }
		if (mGraphicsEngine) { delete mGraphicsEngine; }
		if (mInputManager) { delete mInputManager; }
		if (mWindowSystem) { delete mWindowSystem; }
		if (mEventManager) { delete mEventManager; }
		if (mTaskManager) { delete mTaskManager; }
		SOMBRA_INFO_LOG << "Application deleted";
	}


	void Application::run()
	{
		SOMBRA_INFO_LOG << "Starting the Application";
		mState = AppState::Running;

		utils::TaskSet initialTaskSet(*mTaskManager, false);
		initialTaskSet.createTask([this]() {
			auto currentTp = std::chrono::high_resolution_clock::now();
			frameTask(currentTp);
		});
		initialTaskSet.submit();

		mTaskManager->run();

		mState = AppState::Stopped;
		SOMBRA_INFO_LOG << "Application stopped";
	}


	void Application::stop()
	{
		SOMBRA_INFO_LOG << "Stopping the Application";

		if (mState == AppState::Running) {
			mStopRunning = true;
		}
	}

// Private functions
	void Application::frameTask(std::chrono::high_resolution_clock::time_point lastTp)
	{
		if (mStopRunning) { return; }

		auto currentTp = std::chrono::high_resolution_clock::now();

		std::chrono::duration<float> durationInSeconds = currentTp - lastTp;
		float deltaTime = durationInSeconds.count();
		/*float waitTime = mUpdateTime - deltaTime;
		if (waitTime > 0) {
			SOMBRA_DEBUG_LOG << "Wait " << waitTime << " seconds";
			std::this_thread::sleep_for( std::chrono::duration<float>(waitTime) );
		}*/

		utils::TaskSet taskSet(*mTaskManager, false);

		auto renderTask = taskSet.createSubTaskSet([this](auto& subTaskSet) { render(subTaskSet); });
		auto inputTask = taskSet.createSubTaskSet([this](auto& subTaskSet) { input(subTaskSet); });
		auto updateTask = taskSet.createSubTaskSet([this, deltaTime](auto& subTaskSet) { update(deltaTime, subTaskSet); });
		auto nextFrameTask = taskSet.createTask([this, currentTp]() { frameTask(currentTp); });

		taskSet.depends(updateTask, renderTask);
		taskSet.depends(updateTask, inputTask);
		taskSet.depends(nextFrameTask, updateTask);

		taskSet.submit();
	}


	void Application::input(utils::SubTaskSet& subTaskSet)
	{
		SOMBRA_DEBUG_LOG << "Init";
		auto windowTask = subTaskSet.createTask([&]() { mWindowSystem->update(); }, 0);
		auto inputTask = subTaskSet.createTask([&]() { mInputManager->update(); });

		subTaskSet.depends(inputTask, windowTask);
		SOMBRA_DEBUG_LOG << "End";
	}


	void Application::update(float deltaTime, utils::SubTaskSet& subTaskSet)
	{
		SOMBRA_DEBUG_LOG << "Init (" << deltaTime << ")";
		auto animationTask = subTaskSet.createTask([&]() { mAnimationManager->update(deltaTime); });
		auto dynamicsTask = subTaskSet.createTask([&]() { mPhysicsManager->doDynamics(deltaTime); });
		auto collisionTask = subTaskSet.createTask([&]() { mCollisionManager->update(deltaTime); });
		auto constraintsTask = subTaskSet.createTask([&]() { mPhysicsManager->doConstraints(deltaTime); });
		auto audioTask = subTaskSet.createTask([&]() { mAudioManager->update(); });
		auto graphicsTask = subTaskSet.createTask([&]() { mGraphicsManager->update(); }, 0);

		subTaskSet.depends(collisionTask, dynamicsTask);
		subTaskSet.depends(constraintsTask, collisionTask);
		subTaskSet.depends(audioTask, constraintsTask);
		subTaskSet.depends(audioTask, animationTask);
		subTaskSet.depends(graphicsTask, constraintsTask);
		subTaskSet.depends(graphicsTask, animationTask);
		SOMBRA_DEBUG_LOG << "End";
	}


	void Application::render(utils::SubTaskSet& subTaskSet)
	{
		SOMBRA_DEBUG_LOG << "Init";
		auto renderTask = subTaskSet.createTask([&]() { mGraphicsManager->render(); }, 0);
		auto swapTask = subTaskSet.createTask([&]() { mWindowSystem->swapBuffers(); });

		subTaskSet.depends(swapTask, renderTask);
		SOMBRA_DEBUG_LOG << "End";
	}

}
