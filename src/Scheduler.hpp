#pragma once

#include "Dispatcher.hpp"
#include <unordered_map>
#include <boost/asio.hpp>

class SchedulerTask : public Task
{
public:
    SchedulerTask(std::chrono::nanoseconds delay, TaskFunction&& f)
        : Task(std::forward<TaskFunction>(f))
        , delay(delay)
    {}

    void setEventId(uint32_t id)
    {
        eventId = id;
    }
    uint32_t getEventId() const
    {
        return eventId;
    }

    std::chrono::nanoseconds getDelay() const
    {
        return delay;
    }
private:
    std::chrono::nanoseconds delay;
    uint32_t eventId{ 0 };

};

class Scheduler : public ThreadRunner<Scheduler>, public std::enable_shared_from_this<Scheduler>
{
public:
    Scheduler(std::shared_ptr<Dispatcher> dispatcher);

    uint32_t addEvent(std::unique_ptr<SchedulerTask> task);
    void stopEvent(uint32_t eventId);

    void shutdown();
    void threadMain() { io_context.run(); }

private:
    std::atomic<uint32_t> lastEventId{0};
    std::unordered_map<uint32_t, boost::asio::steady_timer> eventIdTimerMap;
    boost::asio::io_context io_context;
    boost::asio::io_context::work work{ io_context };
    std::shared_ptr<Dispatcher> dispatcher;
};
