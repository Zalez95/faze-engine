#ifndef APPLICATION_H
#define APPLICATION_H

#include <chrono>

namespace se::window { struct WindowData; class WindowSystem; }
namespace se::graphics { struct GraphicsData; class GraphicsEngine; }
namespace se::physics { class PhysicsEngine; }
namespace se::collision { struct CollisionWorldData; class CollisionWorld; }
namespace se::animation { class AnimationSystem; }
namespace se::audio { class AudioEngine; }
namespace se::utils { class TaskManager; class SubTaskSet; }

namespace se::app {

	class EventManager;
	class InputManager;
	class GraphicsManager;
	class PhysicsManager;
	class CollisionManager;
	class AnimationManager;
	class AudioManager;
	class GUIManager;


	/**
	 * Class Application, it's the class that every App must inherit from
	 * to get access to all the SOMBRA managers and systems and to be updated
	 * at a constant rate.
	 */
	class Application
	{
	protected:	// Nested Types
		/** The different states in which the Application could be */
		enum class AppState
		{
			Error,
			Running,
			Stopped
		};

	protected:	// Attributes
		static constexpr int kMaxTasks				= 1024;
		static constexpr float kBaseBias			= 0.1f;
		static constexpr float kMinFDifference		= 0.00001f;
		static constexpr float kContactPrecision	= 0.0000001f;
		static constexpr float kContactSeparation	= 0.00001f;

		/** The minimum elapsed time between updates in seconds */
		const float mUpdateTime;

		/** The state of the Application */
		AppState mState;

		/** The vairable used for stopping the main loop */
		bool mStopRunning;

		window::WindowSystem* mWindowSystem;
		graphics::GraphicsEngine* mGraphicsEngine;
		physics::PhysicsEngine* mPhysicsEngine;
		collision::CollisionWorld* mCollisionWorld;
		animation::AnimationSystem* mAnimationSystem;
		audio::AudioEngine* mAudioEngine;
		utils::TaskManager* mTaskManager;

		/** The managers that hold the data of the entities */
		EventManager* mEventManager;
		InputManager* mInputManager;
		GraphicsManager* mGraphicsManager;
		PhysicsManager* mPhysicsManager;
		CollisionManager* mCollisionManager;
		AnimationManager* mAnimationManager;
		AudioManager* mAudioManager;
		GUIManager* mGUIManager;

	public:		// Functions
		/** Creates a new Application
		 *
		 * @param	windowConfig the initial configuration with which the
		 *			window is going to be created
		 * @param	graphicsConfig the initial configuration with which the
		 *			GraphicsEngine is going to be created
		 * @param	collisionConfig the initial configuration with which the
		 *			CollisionWorld is going to be created
		 * @param	updateTime the minimum elapsed time between updates in
		 *			seconds */
		Application(
			const window::WindowData& windowConfig,
			const graphics::GraphicsData& graphicsConfig,
			const collision::CollisionWorldData& collisionConfig,
			float updateTime
		);

		/** Class destructor */
		virtual ~Application();

		/** Function used for running the Application
		 * @note	the current thread will be used by the Application until
		 *			@see stop is called */
		void run();

		/** Function used for stopping the Application */
		void stop();
	private:
		/** The function that is going to be called at each frame. It will also
		 * create a task for executing the next frame until @see mStopRunning
		 * is set to true.
		 *
		 * @param	lastTp the time point of the last frame executed */
		void frameTask(std::chrono::high_resolution_clock::time_point lastTp);
	protected:
		/** Retrieves all the user input
		 *
		 * @param	subTaskSet the SubTaskSet where all the input tasks are
		 *			going to be submitted */
		virtual void input(utils::SubTaskSet& subTaskSet);

		/** Updates all the Application Managers
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds
		 * @param	subTaskSet the SubTaskSet where all the update tasks are
		 *			going to be submitted */
		virtual void update(float deltaTime, utils::SubTaskSet& subTaskSet);

		/** Draws to screen
		 *
		 * @param	subTaskSet the SubTaskSet where all the render tasks are
		 *			going to be submitted */
		virtual void render(utils::SubTaskSet& subTaskSet);
	};

}

#endif		// APPLICATION_H
