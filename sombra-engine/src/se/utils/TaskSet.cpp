#include "se/utils/TaskSet.h"
#include "se/utils/Log.h"

namespace se::utils {

	SubTaskSet::SubTaskSet(
		TaskManager& taskManager, const FuncSTS& initialFunction, const FuncTask& finalFunction, bool join
	) : mTaskManager(taskManager),
		mInitialTaskId(mTaskManager.create()), mInitialTaskFunction(initialFunction),
		mFinalTaskId(mTaskManager.create()), mFinalTaskFunction(finalFunction),
		mJoinTasks(join)
	{
		mTasks.reserve(mTaskManager.getMaxTasks());
		mSubTaskSets.reserve(mTaskManager.getMaxTasks() / 2);

		if (*this) {
			depends(mFinalTaskId, mInitialTaskId);
		}

		SOMBRA_TRACE_LOG << "Set[" << this << "] Created SubTaskSet with InitialTask " << mInitialTaskId
			<< ", FinalTask " << mFinalTaskId << " and join " << mJoinTasks;
	}


	SubTaskSet::operator bool() const
	{
		return (mInitialTaskId >= 0) && (mFinalTaskId >= 0);
	}


	TaskId SubTaskSet::createTask(const FuncTask& function, int threadNumber)
	{
		TaskId taskId = mTasks.emplace_back(mTaskManager.create(function));
		if (taskId >= 0) {
			if (mJoinTasks) {
				depends(mFinalTaskId, taskId);
			}

			if (threadNumber >= 0) {
				mTaskManager.setThreadAffinity(taskId, threadNumber);
			}
		}

		SOMBRA_TRACE_LOG << "Set[" << this << "] Added task " << taskId;
		return taskId;
	}


	SubTaskSet& SubTaskSet::createSubTaskSet(const FuncSTS& function, bool join)
	{
		SubTaskSet& ret = mSubTaskSets.emplace_back(mTaskManager, function, FuncTask(), join);
		if (ret) {
			if (mJoinTasks) {
				depends(mFinalTaskId, ret.mFinalTaskId);
			}
		}

		SOMBRA_TRACE_LOG << "Set[" << this << "] Added SubTaskSet with InitialTask " << ret.mInitialTaskId;
		return ret;
	}


	void SubTaskSet::depends(TaskId taskId1, TaskId taskId2)
	{
		mTaskManager.addDependency(taskId1, taskId2);
	}


	void SubTaskSet::depends(const SubTaskSet& subSet1, TaskId taskId2)
	{
		mTaskManager.addDependency(subSet1.mInitialTaskId, taskId2);
	}


	void SubTaskSet::depends(TaskId taskId1, const SubTaskSet& subSet2)
	{
		mTaskManager.addDependency(taskId1, subSet2.mFinalTaskId);
	}


	void SubTaskSet::depends(const SubTaskSet& subSet1, const SubTaskSet& subSet2)
	{
		mTaskManager.addDependency(subSet1.mInitialTaskId, subSet2.mFinalTaskId);
	}


	void SubTaskSet::submitSubTaskSetTasks()
	{
		SOMBRA_TRACE_LOG << "Set[" << this << "] Start";

		// Set the initial task function with a copy of the current SubTaskSet
		if (mInitialTaskFunction) {
			mTaskManager.setTaskFunction(mInitialTaskId, [tmp = *this]() mutable {
				tmp.mInitialTaskFunction(tmp);
				tmp.submitCreatedTasks();
			});
		}
		else {
			mTaskManager.setTaskFunction(mInitialTaskId, [tmp = *this]() mutable {
				tmp.submitCreatedTasks();
			});
		}
		mTaskManager.submit(mInitialTaskId);

		// Set the final task function with a copy of the current SubTaskSet
		if (mFinalTaskFunction) {
			mTaskManager.setTaskFunction(mFinalTaskId, [tmp = *this]() {
				tmp.mFinalTaskFunction();
			});
		}
		mTaskManager.submit(mFinalTaskId);

		SOMBRA_TRACE_LOG << "Set[" << this << "] End";
	}


	void SubTaskSet::submitCreatedTasks()
	{
		SOMBRA_TRACE_LOG << "Set[" << this << "] Start";

		for (auto& taskId : mTasks) {
			if (taskId >= 0) {
				mTaskManager.submit(taskId);
			}
		}

		for (auto& subSet : mSubTaskSets) {
			if (subSet) {
				subSet.submitSubTaskSetTasks();
			}
		}

		SOMBRA_TRACE_LOG << "Set[" << this << "] End";
	}


	TaskSet::TaskSet(TaskManager& taskManager, bool join) :
		SubTaskSet(
			taskManager,
			FuncSTS(),
			join? [this]() { mCV.notify_all(); } : FuncTask(),
			join
		) {}


	void TaskSet::submit()
	{
		SOMBRA_TRACE_LOG << "Set[" << this << "] Start";

		SubTaskSet::submitSubTaskSetTasks();

		SOMBRA_TRACE_LOG << "Set[" << this << "] End";
	}


	void TaskSet::submitAndWait()
	{
		SOMBRA_TRACE_LOG << "Set[" << this << "] Start";

		SubTaskSet::submitSubTaskSetTasks();

		std::unique_lock<std::mutex> lock(mMutex);
		mCV.wait(lock);

		SOMBRA_TRACE_LOG << "Set[" << this << "] End";
	}

}
