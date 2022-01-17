#pragma once

#include <boost/asio/io_service.hpp>

class Dispatcher;
class Protocol;
class Session;

class SessionFactory
{
public:
    virtual ~SessionFactory() = default;
    virtual std::shared_ptr<Session> create(std::shared_ptr<Dispatcher> dispatcher, boost::asio::io_service& ioService) = 0;
};
