#pragma once

#include "ServiceBase.hpp"
#include <boost/asio.hpp>
#include "NetworkMessage.hpp"

class ProtocolFactoryBase;
class Dispatcher;
class Session;

class ClientService : public std::enable_shared_from_this<ClientService>, public ServiceBase<ClientService>
{
public:
    ClientService(std::shared_ptr<Dispatcher> dispatcher, boost::asio::io_service& ioService, std::shared_ptr<ProtocolFactoryBase> service);
    void run();
    void open(std::string& ipAddress, uint16_t port);

private:
    void onConnect(const boost::system::error_code& err);

private:
    std::shared_ptr<Session> session;
    std::shared_ptr<ProtocolFactoryBase> service;
    std::shared_ptr<Dispatcher> dispatcher;
    boost::asio::io_service& ioService;
};