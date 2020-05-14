#include <algorithm>
#include "se/utils/TaskManager.h"
#include "se/utils/Log.h"

namespace se::utils {

	TaskManager::TaskManager(int maxTasks, int numThreads) :
		mTasks(maxTasks), mThreads(numThreads - 1, nullptr), mEnd(true)
	{
		SOMBRA_INFO_LOG << "Creating TaskManager with up to " << maxTasks
			<< " tasks and " << numThreads << " threads";

		for (auto& task : mTasks) {
			task.dependentTasks.reserve(maxTasks);
		}
	}


	TaskManager::~TaskManager()
	{
		SOMBRA_INFO_LOG << "Destroying TaskManager";

		if (!mEnd) {
			stop();
		}

		SOMBRA_INFO_LOG << "TaskManager destroyed";
	}


	void TaskManager::run()
	{
		SOMBRA_TRACE_LOG << "Starting the TaskManager";

		if (!mEnd) {
			SOMBRA_ERROR_LOG << "TaskManager must be stopped before running again";
			return;
		}

		mEnd = false;

		// Create the other threads
		int threadNumber = 1;
		for (auto& th : mThreads) {
			th = new std::thread([this, threadNumber]() { thRun(threadNumber); });
			threadNumber++;
		}

		// Run as thread 0
		thRun(0);

		// Destroy the other threads
		for (auto& th : mThreads) {
			if (th && th->joinable()) {
				th->join();
				delete th;
				th = nullptr;
			}
		}

		SOMBRA_TRACE_LOG << "TaskManager stopped";
	}


	void TaskManager::stop()
	{
		SOMBRA_TRACE_LOG << "Stopping the TaskManager";

		{
			std::unique_lock lck(mMutex);
			mEnd = true;
		}
		mCV.notify_all();
	}


	TaskId TaskManager::create(const TaskFunction& function)
	{
		TaskId taskId = -1;

		for (TaskId taskId2 = 0; taskId2 < static_cast<TaskId>(mTasks.size()); ++taskId2) {
			while (mTasks[taskId2].lock.test_and_set(std::memory_order_acquire));

			if (mTasks[taskId2].state == TaskState::Released) {
				mTasks[taskId2].state = TaskState::Created;
				mTasks[taskId2].function = function;
				mTasks[taskId2].threadAffinity = -1;
				mTasks[taskId2].lock.clear(std::memory_order_release);

				taskId = taskId2;
				break;
			}

			mTasks[taskId2].lock.clear(std::memory_order_release);
		}

		if (taskId >= 0) {
			SOMBRA_TRACE_LOG << "Created Task " << taskId;
		}
		else {
			SOMBRA_WARN_LOG << "Can't create more tasks";
		}

		return taskId;
	}


	void TaskManager::setTaskFunction(TaskId taskId, const TaskFunction& function)
	{
		while (mTasks[taskId].lock.test_and_set(std::memory_order_acquire));

		if ((mTasks[taskId].state == TaskState::Created) || (mTasks[taskId].state == TaskState::Submitted)) {
			mTasks[taskId].function = function;
		}
		else {
			SOMBRA_WARN_LOG << "Can't set the function of Task " << taskId;
		}

		mTasks[taskId].lock.clear(std::memory_order_release);
	}


	void TaskManager::setThreadAffinity(TaskId taskId, int threadNumber)
	{
		if (threadNumber < getNumThreads()) {
			while (mTasks[taskId].lock.test_and_set(std::memory_order_acquire));

			if ((mTasks[taskId].state == TaskState::Created) || (mTasks[taskId].state == TaskState::Submitted)) {
				mTasks[taskId].threadAffinity = threadNumber;
			}

			mTasks[taskId].lock.clear(std::memory_order_release);

			SOMBRA_TRACE_LOG << "Added thread " << threadNumber << " affinity to Task " << taskId;
		}
		else {
			SOMBRA_WARN_LOG << "Can't add thread " << threadNumber << " affinity to Task " << taskId;
		}
	}


	void TaskManager::addDependency(TaskId taskId1, TaskId taskId2)
	{
		while (mTasks[taskId1].lock.test_and_set(std::memory_order_acquire));
		while (mTasks[taskId2].lock.test_and_set(std::memory_order_acquire));

		auto itDependent = std::find(
			mTasks[taskId2].dependentTasks.begin(), mTasks[taskId2].dependentTasks.end(),
			taskId1
		);
		if (((mTasks[taskId1].state == TaskState::Created) || (mTasks[taskId1].state == TaskState::Submitted))
			&& ((mTasks[taskId2].state == TaskState::Created) || (mTasks[taskId2].state == TaskState::Submitted))
			&& (itDependent == mTasks[taskId2].dependentTasks.end())
		) {
			mTasks[taskId1].remainingTasks++;
			mTasks[taskId2].dependentTasks.push_back(taskId1);
			SOMBRA_TRACE_LOG << "Added dependency between Tasks " << taskId1 << " and " << taskId2;
		}
		else {
			SOMBRA_WARN_LOG << "Can't add dependency between Tasks " << taskId1 << " and " << taskId2;
		}

		mTasks[taskId2].lock.clear(std::memory_order_release);
		mTasks[taskId1].lock.clear(std::memory_order_release);
	}


	void TaskManager::submit(TaskId taskId)
	{
		while (mTasks[taskId].lock.test_and_set(std::memory_order_acquire));
		if (mTasks[taskId].state == TaskState::Created) {
			mTasks[taskId].state = TaskState::Submitted;
			mTasks[taskId].lock.clear(std::memory_order_release);

			// Push the taskId to mWorkingQueue and notify so it can be executed
			{
				std::unique_lock lck(mMutex);
				mWorkingQueue.push_back(taskId);
			}
			mCV.notify_one();

			SOMBRA_TRACE_LOG << "Submitted Task " << taskId;
		}
		else {
			mTasks[taskId].lock.clear(std::memory_order_release);
			SOMBRA_WARN_LOG << "Can't submit Task " << taskId;
		}
	}

// Private functions
	void TaskManager::thRun(int threadNumber)
	{
		SOMBRA_INFO_LOG << "Thread " << threadNumber << " start";

		std::unique_lock lck(mMutex);
		while (!mEnd) {
			TaskId taskId = getTaskId(threadNumber);
			if (taskId >= 0) {
				lck.unlock();

				SOMBRA_TRACE_LOG << "Executing task " << taskId;
				if (mTasks[taskId].function) {
					mTasks[taskId].function();
				}
				releaseTask(taskId);
				SOMBRA_TRACE_LOG << "Released task " << taskId;

				lck.lock();
			}
			else {
				mCV.wait(lck);
			}
		}

		SOMBRA_INFO_LOG << "Thread " << threadNumber << " end";
	}


	TaskId TaskManager::getTaskId(int threadNumber)
	{
		TaskId taskId = -1;

		// Find a Task in the Queue that is in a Submitted state, has 0
		// remaining tasks and can be executed in the curren thread
		for (std::size_t i = 0; i < mWorkingQueue.size();) {
			TaskId taskId2 = mWorkingQueue[i];

			while (mTasks[taskId2].lock.test_and_set(std::memory_order_acquire));

			if ((i == 0) && (mTasks[taskId2].state == TaskState::Released)) {
				mWorkingQueue.pop_front();
			}
			else if (
				(mTasks[taskId2].state == TaskState::Submitted)
				&& (mTasks[taskId2].remainingTasks == 0)
				&& ((mTasks[taskId2].threadAffinity < 0) || (mTasks[taskId2].threadAffinity == threadNumber))
			) {
				mTasks[taskId2].state = TaskState::Running;
				mTasks[taskId2].lock.clear(std::memory_order_release);

				taskId = taskId2;
				break;
			}
			else {
				i++;
			}

			mTasks[taskId2].lock.clear(std::memory_order_release);
		}

		return taskId;
	}


	void TaskManager::releaseTask(TaskId taskId)
	{
		while (mTasks[taskId].lock.test_and_set(std::memory_order_acquire));

		mTasks[taskId].state = TaskState::Released;

		// Decrement the dependentTasks' remainingTasks
		for (TaskId dependentTaskId : mTasks[taskId].dependentTasks) {
			while (mTasks[dependentTaskId].lock.test_and_set(std::memory_order_acquire));
			mTasks[dependentTaskId].remainingTasks--;
			mTasks[dependentTaskId].lock.clear(std::memory_order_release);
		}
		mTasks[taskId].dependentTasks.clear();

		mTasks[taskId].lock.clear(std::memory_order_release);

		// Notify so the dependent tasks can be executed
		mCV.notify_all();
	}

}
