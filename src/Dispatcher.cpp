#include "Dispatcher.hpp"

Dispatcher::Dispatcher()
{
	start();
}

Dispatcher::~Dispatcher()
{
    shutdown();
}

void Dispatcher::threadMain()
{
	std::unique_lock<std::mutex> taskLockUnique(taskLock, std::defer_lock);

	while (getState() != ThreadRunnerState::THREAD_STATE_TERMINATED)
    {
		taskLockUnique.lock();
		if (taskList.empty())
        {
			taskSignal.wait(taskLockUnique);
		}
	    std::vector<std::unique_ptr<Task>> toExecuteTaskList;
		toExecuteTaskList.swap(taskList);
		taskLockUnique.unlock();

		for (auto& task : toExecuteTaskList)
        {
			if (!task->hasExpired())
            {
				(*task)();
			}
		}
		toExecuteTaskList.clear();
	}
}


void Dispatcher::addTask(std::unique_ptr<Task> task)
{
	bool doSignal = false;
    {
        std::lock_guard<std::mutex> lock{ taskLock };

        if (getState() == ThreadRunnerState::THREAD_STATE_RUNNING)
        {
            doSignal = taskList.empty();
            taskList.push_back(std::move(task));
        }
    }
	if (doSignal)
    {
		taskSignal.notify_one();
	}
}

void Dispatcher::shutdown()
{
	auto task = createTask([this]() {
		setState(ThreadRunnerState::THREAD_STATE_TERMINATED);
		taskSignal.notify_one();
	});

	std::lock_guard<std::mutex> lockClass(taskLock);
	taskList.push_back(std::move(task));

	taskSignal.notify_one();
}
