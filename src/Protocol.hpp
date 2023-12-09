#pragma once

#include <memory>
#include <vector>

class NetworkMessage;
class BufferWriter;

class Protocol
{
public:
    virtual ~Protocol() = default;
    virtual void start() = 0;
    virtual void onAccept() = 0;
    virtual void onClose() = 0;
    virtual void onRecvMessage(std::vector<uint8_t> data) = 0;
    virtual void onSendMessage(const std::shared_ptr<BufferWriter> &message) = 0;
};