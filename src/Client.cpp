#include "Client.hpp"
#include "Dispatcher.hpp"
#include "ProtocolFactory.hpp"
#include "Session.hpp"
#include <iostream>

ClientService::ClientService(std::shared_ptr<Dispatcher> dispatcher, boost::asio::io_service& ioService, std::shared_ptr<ProtocolFactoryBase> service)
    : dispatcher{std::move(dispatcher)}
    , ioService(ioService)
    , session{std::make_shared<Session>(dispatcher, ioService)}
    , service{std::move(service)}
{
}


void ClientService::run()
{

}

void ClientService::open(std::string& ipAddress, uint16_t port)
{
    session->getSocket().async_connect(
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address(boost::asio::ip::address_v4::from_string(ipAddress)), port),
        std::bind(&ClientService::onConnect, shared_from_this(), std::placeholders::_1)
    );
}

void ClientService::onConnect(const boost::system::error_code& err)
{
    if (err)
    {
        std::cout << "Error to connect: " << err.message() << std::endl;
        return;
    }

    std::cout << "Connected succesfully" << std::endl;
    session->accept(service->createProtocol(session));
    session->read();
}