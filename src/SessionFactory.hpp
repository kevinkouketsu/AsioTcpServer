#pragma once

#include <boost/asio/io_service.hpp>

template<typename SessionType>
class SessionFactory
{
public:
    virtual ~SessionFactory() = default;
    virtual std::shared_ptr<SessionType> create(boost::asio::io_service& ioService) = 0;
};
