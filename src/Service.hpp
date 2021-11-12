#pragma once

#include "Protocol.hpp"
#include "ThreadRunner.hpp"
#include <boost/asio.hpp>
#include <string>
#include <type_traits>
#include <unordered_map>

class Session;
class Dispatcher;
class TcpService;
class ProtocolBase;

class ProtocolFactoryBase
{
public:
    virtual ~ProtocolFactoryBase() = default;
    virtual std::shared_ptr<Protocol> createProtocol(std::shared_ptr<Session> session) = 0;
};

template<typename ProtocolType, typename = typename std::enable_if<std::is_base_of<Protocol, ProtocolType>::value>::type>
class ProtocolFactory : public ProtocolFactoryBase
{
public:
    std::shared_ptr<Protocol> createProtocol(std::shared_ptr<Session> session) override
    {
        return std::make_shared<ProtocolType>(session);
    }
};

class Services : public std::enable_shared_from_this<Services>
{
public:
    Services(std::shared_ptr<Dispatcher> dispatcher);

    template<typename ServiceType>
    void add(int16_t port, std::string ipAddress)
    {
        auto service = std::make_shared<TcpService>(dispatcher, ioService, std::make_shared<Service<ServiceType>>());
        service->open(ipAddress, port);
        services[port] = std::move(service);
    }

    void run()
    {
        ioService.run();
    }

private:
    std::shared_ptr<Dispatcher> dispatcher;
    boost::asio::io_service ioService;
    std::unordered_map<int16_t, std::shared_ptr<TcpService>> services;
};

class TcpService : public std::enable_shared_from_this<TcpService>
{
public:
    TcpService(std::shared_ptr<Dispatcher> dispatcher, boost::asio::io_service& ioService, std::shared_ptr<ProtocolFactoryBase> service);
    void open(std::string ipAddress, int16_t port);

private:
    void accept();
    void onAccept(std::shared_ptr<Session> session, const boost::system::error_code& error);

    boost::asio::io_service& ioService;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    std::shared_ptr<ProtocolFactoryBase> service;
    std::shared_ptr<Dispatcher> dispatcher;
};
