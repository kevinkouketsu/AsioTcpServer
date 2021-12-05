#include "Client.hpp"
#include "Dispatcher.hpp"
#include "ProtocolFactory.hpp"
#include "Session.hpp"
#include "ILogger.hpp"
#include <iostream>

ClientService::ClientService(std::shared_ptr<Dispatcher> dispatcher, std::shared_ptr<Scheduler> scheduler, boost::asio::io_service& ioService, std::shared_ptr<ProtocolFactoryBase> service)
    : dispatcher{std::move(dispatcher)}
    , scheduler{std::move(scheduler)}
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
        NETWORK_LOG_ERROR("Error when trying to connect: " << err.message());
        return;
    }

    NETWORK_LOG_INFO("Connected successfully");
    session->accept(service->createProtocol(dispatcher, scheduler, session));
    session->read();
}
