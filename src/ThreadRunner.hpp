#pragma once

#include <atomic>
#include <thread>

enum class ThreadRunnerState
{
	THREAD_STATE_RUNNING,
	THREAD_STATE_CLOSING,
	THREAD_STATE_TERMINATED,
};

// This class uses static polymorphism (CRTP)
template<typename Derived>
class ThreadRunner
{
public:
    ThreadRunner() = default;
    virtual ~ThreadRunner() = default;

    void join()
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
protected:
    void start()
    {
        setState(ThreadRunnerState::THREAD_STATE_RUNNING);
        thread = std::thread(&Derived::threadMain, static_cast<Derived*>(this));
    }

    void stop()
    {
        setState(ThreadRunnerState::THREAD_STATE_CLOSING);
    }

    ThreadRunnerState getState() const
    {
        return currentState;
    }
    void setState(ThreadRunnerState newState)
    {
        currentState = newState;
    }

private:
    std::thread thread;
    std::atomic<ThreadRunnerState> currentState;
};