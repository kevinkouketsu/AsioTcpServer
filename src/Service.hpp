#pragma once

#include "Protocol.hpp"
#include "ServiceBase.hpp"
#include "ThreadRunner.hpp"
#include "ProtocolFactory.hpp"
#include <boost/asio.hpp>
#include <string>
#include <type_traits>
#include <unordered_map>

class Session;
class Dispatcher;
class TcpService;

class Services : public std::enable_shared_from_this<Services>
{
public:
    Services(std::shared_ptr<Dispatcher> dispatcher, std::shared_ptr<Scheduler> scheduler);

    template<typename ServiceType>
    void add(int16_t port, std::string ipAddress)
    {
        auto service = std::make_shared<TcpService>(dispatcher, scheduler, ioService, std::make_shared<ProtocolFactory<ServiceType>>());
        service->open(ipAddress, port);
        services[port] = std::move(service);
    }

    void run()
    {
        ioService.run();
    }

    bool isRunning()
    {
        return services.empty() == false;
    }

    boost::asio::io_service& getIoService()
    {
        return ioService;
    }
private:
    std::shared_ptr<Dispatcher> dispatcher;
    std::shared_ptr<Scheduler> scheduler;
    boost::asio::io_service ioService;
    std::unordered_map<int16_t, std::shared_ptr<TcpService>> services;
};

class TcpService : public std::enable_shared_from_this<TcpService>, public ServiceBase<TcpService>
{
public:
    TcpService(std::shared_ptr<Dispatcher> dispatcher, std::shared_ptr<Scheduler> scheduler, boost::asio::io_service& ioService, std::shared_ptr<ProtocolFactoryBase> service);
    void open(std::string ipAddress, int16_t port);

private:
    void accept();
    void onAccept(std::shared_ptr<Session> session, const boost::system::error_code& error);

    boost::asio::io_service& ioService;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    std::shared_ptr<ProtocolFactoryBase> service;
    std::shared_ptr<Dispatcher> dispatcher;
    std::shared_ptr<Scheduler> scheduler;
};
