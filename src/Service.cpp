#include "Service.hpp"
#include "Session.hpp"
#include <iostream>

Services::Services(std::shared_ptr<Dispatcher> dispatcher)
    : dispatcher{std::move(dispatcher)}
{
}

TcpService::TcpService(std::shared_ptr<Dispatcher> dispatcher, boost::asio::io_service& ioService, std::shared_ptr<ProtocolFactoryBase> service)
    : ioService{ioService}
    , dispatcher{std::move(dispatcher)}
    , service{std::move(service)}
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
    // TODO: manage the connection correctly
    auto connection = std::make_shared<Session>(dispatcher, ioService);
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
    std::cout << "Accepted a new connection" << std::endl;
    if (!error)
    {
        session->accept(this->service->createProtocol(session));
        session->read();

        // recursively await a new connection
        accept();
    }
    else
    {
        session->close();
    }
}