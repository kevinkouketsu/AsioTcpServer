#pragma once

#include <memory>
#include <type_traits>
#include "Protocol.hpp"

class Session;
class Dispatcher;
class Scheduler;

template <typename T>
class ProtocolFactoryBase
{
public:
    virtual ~ProtocolFactoryBase() = default;
    virtual std::shared_ptr<Protocol<T>> createProtocol(std::shared_ptr<Dispatcher> dispatcher, std::shared_ptr<Scheduler> scheduler, std::shared_ptr<T> session) = 0;
};

template<typename ProtocolType, typename SessionType, typename = typename std::enable_if<std::is_base_of<Protocol<SessionType>, ProtocolType>::value>::type>
class ProtocolFactory : public ProtocolFactoryBase<SessionType>
{
public:
    std::shared_ptr<Protocol<SessionType>> createProtocol(std::shared_ptr<Dispatcher> dispatcher, std::shared_ptr<Scheduler> scheduler, std::shared_ptr<SessionType> session) override
    {
        return std::make_shared<ProtocolType>(dispatcher, scheduler, session);
    }
};
