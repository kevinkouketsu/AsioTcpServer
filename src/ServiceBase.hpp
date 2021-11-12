#pragma once

template<typename Derived>
class ServiceBase : public std::shared_from_this<ServiceBase>
{
public:
    void run()
    {
        static_cast<Derived*>(this)->run();
    }

};