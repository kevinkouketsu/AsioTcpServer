#pragma once

#include <memory>

class NetworkMessage;
class BufferWriter;

class Protocol
{
public:
    virtual ~Protocol() = default;
    virtual void onAccept() = 0;
    virtual void onClose() = 0;
    virtual void onRecvMessage(NetworkMessage& msg) = 0;
    virtual void onSendMessage(const std::shared_ptr<BufferWriter>& message) = 0;
};