#include "Service.hpp"
#include "Session.hpp"
#include <iostream>
#include "ILogger.hpp"

class DefaultSessionFactory : public SessionFactory
{
public:
    std::shared_ptr<Session> create(std::shared_ptr<Dispatcher> dispatcher, boost::asio::io_service& ioService)
    {
        return std::make_shared<Session>(dispatcher, ioService);
    }
};

Services::Services(std::shared_ptr<Dispatcher> dispatcher, std::shared_ptr<Scheduler> scheduler)
    : Services(dispatcher, scheduler, std::make_shared<DefaultSessionFactory>())
{
}

Services::Services(std::shared_ptr<Dispatcher> dispatcher, std::shared_ptr<Scheduler> scheduler, std::shared_ptr<SessionFactory> sessionFactory)
    : dispatcher{std::move(dispatcher)}
    , scheduler{std::move(scheduler)}
    , sessionFactory{std::move(sessionFactory)}
{
}

TcpService::TcpService(std::shared_ptr<Dispatcher> dispatcher, std::shared_ptr<Scheduler> scheduler, boost::asio::io_service& ioService, std::shared_ptr<ProtocolFactoryBase> protocolFacotry)
    : ioService{ioService}
    , dispatcher{std::move(dispatcher)}
    , scheduler{std::move(scheduler)}
    , protocolFacotry{std::move(protocolFacotry)}
{
}

void TcpService::open(std::string ipAddress, int16_t port)
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

void TcpService::accept()
{
    auto connection = this->sessionFactory->create(dispatcher, ioService);
	acceptor->async_accept(
        connection->getSocket(),
        std::bind(
            &TcpService::onAccept,
            shared_from_this(),
            connection,
            std::placeholders::_1
        )
    );
}

void TcpService::onAccept(std::shared_ptr<Session> session, const boost::system::error_code& error)
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
