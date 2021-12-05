#pragma once

#include "ThreadRunner.hpp"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>

using TaskFunction = std::function<void()>;

class Task
{
public:
    explicit Task(TaskFunction&& func)
        : func{std::move(func)}
    {}

    Task(std::chrono::nanoseconds delay, TaskFunction func)
        : expiration(std::chrono::steady_clock::now() + delay)
        , func{std::move(func)}
    {}

    virtual ~Task() = default;
    void operator()()
    {
        func();
    }

    bool hasExpired() const
    {
        if (expiration == std::chrono::steady_clock::time_point{})
        {
            return false;
        }
        return expiration < std::chrono::steady_clock::now();
    }
private:
    std::chrono::steady_clock::time_point expiration{};
    TaskFunction func;
};

template<typename ClassType = Task>
std::unique_ptr<ClassType> createTask(std::chrono::nanoseconds delay, TaskFunction&& func)
{
    return std::make_unique<ClassType>(delay, std::move(func));
}

template<typename ClassType = Task>
std::unique_ptr<ClassType> createTask(TaskFunction&& func)
{
    return std::make_unique<ClassType>(std::move(func));
}

class Dispatcher : public ThreadRunner<Dispatcher>
{
public:
    Dispatcher();
    ~Dispatcher();

    void threadMain();
    void addTask(std::unique_ptr<Task> task);
    void shutdown();

private:
    std::mutex taskLock;
    std::condition_variable taskSignal;
    size_t maximumTasks { 0 };
    std::vector<std::unique_ptr<Task>> taskList;
};
