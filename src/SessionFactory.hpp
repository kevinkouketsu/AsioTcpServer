#pragma once

#include <boost/asio/io_service.hpp>

class Dispatcher;
class Protocol;

template<typename T>
class SessionFactory
{
public:
    virtual ~SessionFactory() = default;
    virtual std::shared_ptr<T> create(std::shared_ptr<Dispatcher> dispatcher, boost::asio::io_service& ioService) = 0;
};
