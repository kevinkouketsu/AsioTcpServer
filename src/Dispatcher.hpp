#pragma once

#include "ThreadRunner.hpp"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>

class Task;
std::unique_ptr<Task> createTask(std::function<void()> func);

template<typename Rep, typename Period>
std::unique_ptr<Task> createTask(std::chrono::duration<Rep, Period> delay, std::function<void()> func)
{
    return std::make_unique<Task>(delay, std::move(func));
}

class Task
{
public:
    explicit Task(std::function<void()> func)
        : func{std::move(func)}
    {}

    template<typename Rep, typename Period>
    Task(std::chrono::duration<Rep, Period> delay, std::function<void()> func)
        : expiration(std::chrono::system_clock::now() + delay)
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
    std::function<void()> func;
};

class Dispatcher : public ThreadRunner<Dispatcher>
{
public:
    void threadMain();
    ~Dispatcher();

    void addTask(std::unique_ptr<Task> task);
    void shutdown();

private:

    std::mutex taskLock;
    std::condition_variable taskSignal;

    std::vector<std::unique_ptr<Task>> taskList;
};