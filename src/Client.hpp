#pragma once

#include "ServiceBase.hpp"
#include <boost/asio.hpp>
#include "NetworkMessage.hpp"
#include "ProtocolFactory.hpp"
#include "SessionFactory.hpp"
#include "Session.hpp"
#include "Dispatcher.hpp"
#include "Scheduler.hpp"
#include "ILogger.hpp"

template<typename T = Session>
class ClientService : public std::enable_shared_from_this<ClientService<T>>, public ServiceBase<ClientService<T>>
{
public:
    ClientService(std::shared_ptr<Dispatcher> dispatcher, std::shared_ptr<Scheduler> scheduler, boost::asio::io_service& ioService, std::shared_ptr<ProtocolFactoryBase<T>> service)
        : ClientService<T>(dispatcher, scheduler, ioService, service, std::make_shared<DefaultSessionFactory>(dispatcher, ioService))
    {}
    ClientService(std::shared_ptr<Dispatcher> dispatcher, std::shared_ptr<Scheduler> scheduler, boost::asio::io_service& ioService, std::shared_ptr<ProtocolFactoryBase<T>> service, std::shared_ptr<SessionFactory<T>> sessionFactory)
        : dispatcher{std::move(dispatcher)}
        , scheduler{std::move(scheduler)}
        , ioService(ioService)
        , session{sessionFactory->create(dispatcher, ioService)}
        , service{std::move(service)}
    {}

    void run();
    void open(std::string& ipAddress, uint16_t port)
    {
        this->session->getSocket().async_connect(
            boost::asio::ip::tcp::endpoint(boost::asio::ip::address(boost::asio::ip::address_v4::from_string(ipAddress)), port),
            std::bind(&ClientService::onConnect, this->shared_from_this(), std::placeholders::_1)
        );
    }

private:
    void onConnect(const boost::system::error_code& err)
    {
        if (err)
        {
            NETWORK_LOG_ERROR("Error when trying to connect: " << err.message());
            return;
        }

        NETWORK_LOG_INFO("Connected successfully");
        session->accept(service->createProtocol(dispatcher, scheduler, session));
        session->read();
    }

private:
    std::shared_ptr<T> session;
    std::shared_ptr<Scheduler> scheduler;
    std::shared_ptr<ProtocolFactoryBase<T>> service;
    std::shared_ptr<Dispatcher> dispatcher;
    boost::asio::io_service& ioService;
};