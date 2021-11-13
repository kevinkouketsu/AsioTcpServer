#pragma once

#include <memory>
#include <string>

template<typename Derived>
class ServiceBase
{
public:
    void run()
    {
        static_cast<Derived*>(this)->run();
    }
    void open(std::string& ipAddress, uint16_t port)
    {
        static_cast<Derived*>(this)->open(ipAddress, port);
    }
};