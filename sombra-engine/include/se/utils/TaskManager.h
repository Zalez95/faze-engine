#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <deque>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace se::utils {

	/** The id used for identifying the Tasks in the TaskManager */
	using TaskId = int;


	/**
	 * Class TaskManager, it's used for executing tasks in a given order in
	 * parallel
	 */
	class TaskManager
	{
	private:	// Nested types
		using TaskFunction = std::function<void()>;

		/** The different states in which a Task can be */
		enum class TaskState { Created, Submitted, Running, Released };

		/** Struct Task, holds a function to execute in some thread when its
		 * task dependencies are finished */
		struct Task
		{
			/** The current state of the Task */
			TaskState state = TaskState::Released;

			/** The function to execute */
			TaskFunction function;

			/** The number of task dependencies of the current one */
			int remainingTasks = 0;

			/** The tasks that depends on the current one */
			std::vector<TaskId> dependentTasks;

			/** The thread where the task must be executed, if it's less than 0
			 * the Task doesn't have thread affinity */
			int threadAffinity = -1;

			/** Atomic flag used as lock for accessing to all the properties */
			std::atomic_flag lock = ATOMIC_FLAG_INIT;
		};

	private:	// Attributes
		/** All the tasks objects of the TaskManager */
		std::vector<Task> mTasks;

		/** All the threads of the TaskManager */
		std::vector<std::thread*> mThreads;

		/** Bool used for stopping all the threads */
		bool mEnd;

		/** The TaskIds of the Tasks that has been submitted to be executed by
		 * the threads */
		std::deque<TaskId> mWorkingQueue;

		/** Mutex used for accessing to @see mEnd and @see mWorkingQueue */
		std::mutex mMutex;

		/** The condition variable used by the threads for waiting until new
		 * tasks are ready to be executed */
		std::condition_variable mCV;

	public:		// Functions
		/** Creates a new TaskManager
		 *
		 * @param	maxTasks the maximum number of Tasks that can be created
		 * @param	numThreads the number of execution threads of the
		 *			TaskManager. It's value by default is the number of harware
		 *			threads */
		TaskManager(
			int maxTasks,
			int numThreads = std::thread::hardware_concurrency()
		);

		/** Class destructor. It will stop all the threads */
		~TaskManager();

		/** @return	the maximum number of tasks that can be created with the
		 *			TaskManager */
		int getMaxTasks() const
		{ return static_cast<int>(mTasks.size()); };

		/** @return	the number of threads of the TaskManager */
		int getNumThreads() const
		{ return static_cast<int>(mThreads.size()) + 1; };

		/** Starts Running the TaskManager
		 *
		 * @note	the current thread will be used as thread number 0 until
		 *			the TaskManager has stopped */
		void run();

		/** Asks the TaskManager to stop */
		void stop();

		/** Creates a new Task
		 *
		 * @param	function the function to call when the task is ready to be
		 *			executed
		 * @return	the id of the new Task, -1 if it couldn't be created
		 * @note	the new Task won't have any dependencies nor thread
		 *			affinity */
		TaskId create(const TaskFunction& function = TaskFunction());

		/** Sets the function to call when the given task is ready to be
		 * executed
		 *
		 * @param	taskId the task to set its function
		 * @param	function the new function to call */
		void setTaskFunction(TaskId taskId, const TaskFunction& function);

		/** Sets the thread where the given task must be executed
		 *
		 * @param	taskId the task to set its affinity
		 * @param	threadNumber the number of the thread where the task must
		 *			be executed
		 * @note	if the given thread number isn't a valid the current
		 *			affinity of the Task won't be changed */
		void setThreadAffinity(TaskId taskId, int threadNumber);

		/** Adds a dependency between the given tasks
		 *
		 * @param	taskId1 the task that has to wait until @see taskId2 has
		 *			been executed
		 * @param	taskId2 the other task */
		void addDependency(TaskId taskId1, TaskId taskId2);

		/** Submits the given task for execution in one of the TaskManager
		 * threads when its dependencies have been satisified
		 *
		 * @param	taskId the task to submit */
		void submit(TaskId taskId);
	private:
		/** Executes the tasks submitted to the @see mWorkingQueue until
		 * @see mEnd is setted to true
		 *
		 * @param	threadNumber the number of the current thread */
		void thRun(int threadNumber);

		/** Returns a taskId from @see mWorkingQueue that is ready to be
		 * executed in the current thread
		 *
		 * @param	threadNumber the number of the current thread
		 * @return	the TaskId of the Task that can be executed, -1 if there's
		 *			no such a Task
		 * @note	the mutex @see mMutex must have been locked before calling
		 *			this function. The Task state will be updated to Running */
		TaskId getTaskId(int threadNumber);

		/** Releases the given Task and notifies the dependent tasks of the
		 * given one that it already finished its job
		 *
		 * @param	taskId the taskId to release */
		void releaseTask(TaskId taskId);
	};

}

#endif		// TASK_MANAGER_H
