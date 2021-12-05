#include "Dispatcher.hpp"
#include "ILogger.hpp"

Dispatcher::Dispatcher()
{
	start();
}

Dispatcher::~Dispatcher()
{
    NETWORK_LOG_DEBUG("Dispatcher::~Dispatcher()");
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

            auto oldMaximum = maximumTasks;
            maximumTasks = std::max(maximumTasks, taskList.size());
            if (oldMaximum != maximumTasks)
            {
                NETWORK_LOG_DEBUG("Maximum tasks in the dispatcher exceeded: " << maximumTasks);
            }
        }
    }
	if (doSignal)
    {
		taskSignal.notify_one();
	}
}

void Dispatcher::shutdown()
{
    NETWORK_LOG_INFO("Shutting down the dispatcher");
	auto task = createTask([this]() {
		setState(ThreadRunnerState::THREAD_STATE_TERMINATED);
		taskSignal.notify_one();
	});

	std::lock_guard<std::mutex> lockClass(taskLock);
	taskList.push_back(std::move(task));

	taskSignal.notify_one();
}
