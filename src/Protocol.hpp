#pragma once

class Protocol
{
public:
    virtual ~Protocol() = default;
    virtual void onAccept() = 0;
    virtual void onClose() = 0;
};