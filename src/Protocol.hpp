#pragma once

#include <memory>
#include <vector>
#include <type_traits>
#include "Session.hpp"

class NetworkMessage;
class BufferWriter;

template<typename SessionType, typename = typename std::enable_if<std::is_base_of<SessionType, Session>::value>>
class Protocol
{
public:
    virtual ~Protocol() = default;
    virtual void start() = 0;
    virtual void onAccept(std::shared_ptr<SessionType> session) = 0;
    virtual void onClose(std::shared_ptr<SessionType> session) = 0;
};