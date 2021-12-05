
#include "Scheduler.hpp"
#include <boost/asio/post.hpp>
#include "ILogger.hpp"

Scheduler::Scheduler(std::shared_ptr<Dispatcher> dispatcher)
    : dispatcher{std::move(dispatcher)}
{
	start();
}

uint32_t Scheduler::addEvent(std::unique_ptr<SchedulerTask> task)
{
	uint32_t eventId = task->getEventId();
	if (task->getEventId() == 0)
    {
		task->setEventId(++lastEventId);
		eventId = task->getEventId();
	}

	boost::asio::post(io_context, [self=shared_from_this(), task=std::move(task)]() mutable {
		auto it = self->eventIdTimerMap.emplace(task->getEventId(), boost::asio::steady_timer{self->io_context});
		auto& timer = it.first->second;

		timer.expires_from_now(task->getDelay());
		timer.async_wait([self, task=std::move(task)](const boost::system::error_code& error) mutable {
			self->eventIdTimerMap.erase(task->getEventId());
			if (error == boost::asio::error::operation_aborted || self->getState() == ThreadRunnerState::THREAD_STATE_TERMINATED)
            {
				return;
			}

            auto baseTask = std::unique_ptr<Task>(task.release());
			self->dispatcher->addTask(std::move(baseTask));
		});
	});

	return eventId;
}

void Scheduler::stopEvent(uint32_t eventId)
{
	if (eventId == 0)
    {
        NETWORK_LOG_DEBUG("Asked to stop eventId 0, ignoring");
		return;
	}

    NETWORK_LOG_DEBUG("Stopping eventId " << eventId);
	boost::asio::post(io_context, [self=shared_from_this(), eventId]()
    {
		auto it = self->eventIdTimerMap.find(eventId);
		if (it != self->eventIdTimerMap.end())
        {
			it->second.cancel();
		}
	});
}

void Scheduler::shutdown()
{
    NETWORK_LOG_DEBUG("Shutting down the Scheduler");
	setState(ThreadRunnerState::THREAD_STATE_TERMINATED);
	boost::asio::post(io_context, [this]() {
		for (auto& it : eventIdTimerMap)
        {
			it.second.cancel();
		}

		io_context.stop();
	});
}

