#include "Server.hpp"
#include <iostream>
#include <boost/asio.hpp>
#include "Dispatcher.hpp"
#include "Scheduler.hpp"
#include "Service.hpp"
#include "Protocol.hpp"

int main(int argc, char* argv[])
{
    class ProtocolTest : public Protocol
    {
    public:
        ProtocolTest(std::shared_ptr<Session> session)
        {
        }
        void onAccept() override
        {
            std::cout << "onAccept=" << this << std::endl;
        }
        void onClose() override
        {
            std::cout << "onClose=" << this << std::endl;
        }
    };
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
        auto services = std::make_shared<Services>(dispatcher);
        services->add<ProtocolTest>(8174, "");
        services->run();

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
