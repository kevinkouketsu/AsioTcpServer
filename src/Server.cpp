#include "Server.hpp"
#include <iostream>
#include <boost/asio.hpp>
#include "Dispatcher.hpp"
#include "Scheduler.hpp"

int main(int argc, char* argv[])
{
    try
    {
        auto dispatcher = std::make_shared<Dispatcher>();
        auto scheduler = std::make_shared<Scheduler>(dispatcher);
        dispatcher->addTask(createTask([](){
            std::cout << "From another thread " << std::this_thread::get_id() << std::endl;
        }));

        std::cout << "From main thread " << std::this_thread::get_id() << std::endl;
        auto eventId = scheduler->addEvent(createTask<SchedulerTask>(std::chrono::seconds(2), [dispatcher]()
        {
            std::cout << "Calling the scheduled task..." << std::endl;
        }));
        scheduler->stopEvent(eventId);
        scheduler->addEvent(createTask<SchedulerTask>(std::chrono::seconds(5), [dispatcher]()
        {
            std::cout << "Calling the scheduled task..." << std::endl;
            dispatcher->shutdown();
        }));

        dispatcher->join();
        scheduler->shutdown();
        scheduler->join();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
