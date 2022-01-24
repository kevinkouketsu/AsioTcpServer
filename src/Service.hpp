#pragma once

#include "Protocol.hpp"
#include "ProtocolFactory.hpp"
#include "ServiceBase.hpp"
#include "SessionFactory.hpp"
#include "ThreadRunner.hpp"
#include <boost/asio.hpp>
#include <string>
#include <type_traits>
#include <unordered_map>
#include "ILogger.hpp"
#include "Session.hpp"
#include "Dispatcher.hpp"
#include <memory>

template<typename T>
class TcpService;

template<typename T = Session>
class Services : public std::enable_shared_from_this<Services<T>>
{
public:
    Services(std::shared_ptr<Dispatcher> dispatcher, std::shared_ptr<Scheduler> scheduler)
        : Services<T>(dispatcher, scheduler, std::make_shared<DefaultSessionFactory>())
    {
    }
    Services(std::shared_ptr<Dispatcher> dispatcher, std::shared_ptr<Scheduler> scheduler, std::shared_ptr<SessionFactory<T>> sessionFactory)
        : dispatcher{std::move(dispatcher)}
        , scheduler{std::move(scheduler)}
        , sessionFactory{std::move(sessionFactory)}
    {}

    template<typename ServiceType>
    void add(int16_t port, std::string ipAddress)
    {
        auto service = std::make_shared<TcpService<T>>(dispatcher, scheduler, ioService, std::make_shared<ProtocolFactory<ServiceType, T>>(), sessionFactory);
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
    std::shared_ptr<SessionFactory<T>> sessionFactory;
    boost::asio::io_service ioService;
    std::unordered_map<int16_t, std::shared_ptr<TcpService<T>>> services;
};

template<typename T>
class TcpService : public std::enable_shared_from_this<TcpService<T>>, public ServiceBase<TcpService<T>>
{
public:
    TcpService(std::shared_ptr<Dispatcher> dispatcher, std::shared_ptr<Scheduler> scheduler, boost::asio::io_service& ioService, std::shared_ptr<ProtocolFactoryBase<T>> protocolFacotry, std::shared_ptr<SessionFactory<T>> sessionFactory)
        : ioService{ioService}
        , dispatcher{std::move(dispatcher)}
        , scheduler{std::move(scheduler)}
        , protocolFacotry{std::move(protocolFacotry)}
        , sessionFactory{std::move(sessionFactory)}
    {}
    void open(std::string ipAddress, int16_t port)
    {
        if (!ipAddress.empty())
        {
            acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(
                ioService,
                boost::asio::ip::tcp::endpoint(boost::asio::ip::address(boost::asio::ip::address_v4::from_string(ipAddress)), port)
            );
        }
        else
        {
            acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(
                ioService,
                boost::asio::ip::tcp::endpoint(boost::asio::ip::address(boost::asio::ip::address_v4(INADDR_ANY)), port)
            );
        }

        // this settings disables the nagle algorithm
        acceptor->set_option(boost::asio::ip::tcp::no_delay(true));
        accept();
    }

private:
    void accept()
    {
        auto connection = this->sessionFactory->create(dispatcher, ioService);
        acceptor->async_accept(
            connection->getSocket(),
            std::bind(
                &TcpService<T>::onAccept,
                this->shared_from_this(),
                connection,
                std::placeholders::_1
            )
        );
    }
    void onAccept(std::shared_ptr<T> session, const boost::system::error_code& error)
    {
        if (!error)
        {
            NETWORK_LOG_INFO("Accepted a new connection");

            session->accept(this->protocolFacotry->createProtocol(dispatcher, scheduler, session));
            session->read();

            // recursively await a new connection
            accept();
        }
        else
        {
            session->close();
        }
    }

    boost::asio::io_service& ioService;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    std::shared_ptr<ProtocolFactoryBase<T>> protocolFacotry;
    std::shared_ptr<SessionFactory<T>> sessionFactory;
    std::shared_ptr<Dispatcher> dispatcher;
    std::shared_ptr<Scheduler> scheduler;
};
