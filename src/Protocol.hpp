#pragma once

class NetworkMessage;

class Protocol
{
public:
    virtual ~Protocol() = default;
    virtual void onAccept() = 0;
    virtual void onClose() = 0;
    virtual void onRecvMessage(NetworkMessage& msg) = 0;
};