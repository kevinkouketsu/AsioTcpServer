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

template <typename SessionType>
class TcpService;

template <typename SessionType = Session>
class Services : public std::enable_shared_from_this<Services<SessionType>>
{
public:
    Services()
        : Services<SessionType>(std::make_shared<DefaultSessionFactory<SessionType>>())
    {
    }
    Services(std::shared_ptr<SessionFactory<SessionType>> sessionFactory)
        : sessionFactory{std::move(sessionFactory)}
    {
    }

    template<typename ProtocolType, typename = std::enable_if_t<std::is_base_of<Protocol<SessionType>, ProtocolType>::value>>
    void add(std::shared_ptr<ProtocolType> protocol, int16_t port, std::string ipAddress)
    {
        auto service = std::make_shared<TcpService<SessionType>>(ioService, protocol, sessionFactory);
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

    boost::asio::io_service &getIoService()
    {
        return ioService;
    }

private:
    std::shared_ptr<SessionFactory<SessionType>> sessionFactory;
    boost::asio::io_service ioService;
    std::unordered_map<int16_t, std::shared_ptr<TcpService<SessionType>>> services;
};

template <typename SessionType>
class TcpService : public std::enable_shared_from_this<TcpService<SessionType>>, public ServiceBase<TcpService<SessionType>>
{
public:
    TcpService(boost::asio::io_service &ioService, std::shared_ptr<Protocol<SessionType>> protocol, std::shared_ptr<SessionFactory<SessionType>> sessionFactory)
        : ioService{ioService}, sessionFactory{std::move(sessionFactory)}, protocol{std::move(protocol)}
    {
    }
    void open(std::string ipAddress, int16_t port)
    {
        if (!ipAddress.empty())
        {
            acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(
                ioService,
                boost::asio::ip::tcp::endpoint(boost::asio::ip::address(boost::asio::ip::address_v4::from_string(ipAddress)), port));
        }
        else
        {
            acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(
                ioService,
                boost::asio::ip::tcp::endpoint(boost::asio::ip::address(boost::asio::ip::address_v4(INADDR_ANY)), port));
        }

        // this settings disables the nagle algorithm
        acceptor->set_option(boost::asio::ip::tcp::no_delay(true));
        accept();
    }

private:
    void accept()
    {
        auto connection = this->sessionFactory->create(ioService);
        acceptor->async_accept(
            connection->getSocket(),
            std::bind(
                &TcpService<SessionType>::onAccept,
                this->shared_from_this(),
                connection,
                std::placeholders::_1
            )
        );
    }
    void onAccept(std::shared_ptr<SessionType> session, const boost::system::error_code &error)
    {
        if (!error)
        {
            NETWORK_LOG_INFO("Accepted a new connection");

            protocol->onAccept(session);
            session->read();

            // recursively await a new connection
            accept();
        }
        else
        {
            session->close();
        }
    }

    boost::asio::io_service &ioService;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    std::shared_ptr<Protocol<SessionType>> protocol;
    std::shared_ptr<SessionFactory<SessionType>> sessionFactory;
};
