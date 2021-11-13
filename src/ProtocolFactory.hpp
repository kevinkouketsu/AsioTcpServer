#pragma once

#include <memory>
#include <type_traits>
#include "Protocol.hpp"

class Session;

class ProtocolFactoryBase
{
public:
    virtual ~ProtocolFactoryBase() = default;
    virtual std::shared_ptr<Protocol> createProtocol(std::shared_ptr<Session> session) = 0;
};

template<typename ProtocolType, typename = typename std::enable_if<std::is_base_of<Protocol, ProtocolType>::value>::type>
class ProtocolFactory : public ProtocolFactoryBase
{
public:
    std::shared_ptr<Protocol> createProtocol(std::shared_ptr<Session> session) override
    {
        return std::make_shared<ProtocolType>(session);
    }
};
