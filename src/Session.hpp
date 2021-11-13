#pragma once

#include <boost/asio.hpp>
#include "NetworkMessage.hpp"
#include "Dispatcher.hpp"

class Dispatcher;
class Protocol;

class Session : public std::enable_shared_from_this<Session>
{
public:
    static constexpr bool ForceClose = true;

    // The SessionHandler lifetime must be the same as the Session
    Session(std::shared_ptr<Dispatcher> dispatcher, boost::asio::io_service& ioService);

    boost::asio::ip::tcp::socket& getSocket();
    uint32_t getIpAddress() const;

    void accept(std::shared_ptr<Protocol> protocol);
    void read();
    void close(bool force = false);

private:
    void parseHeader(const boost::system::error_code& error);
    void parsePacket(const boost::system::error_code& error);
    void closeSocket();
    void handleTimeout(const boost::system::error_code& error);
    void setTimerTimeout(boost::asio::steady_timer& steadyTimer, std::chrono::seconds timeout);
    void parseHelloPacket(const boost::system::error_code& error);

    template<typename T, typename... Args>
    void addTask(T method, Args... args)
    {
        dispatcher->addTask(createTask(std::bind(method, this->protocol, std::forward<Args>(args)...)));
    }
private:

    mutable std::recursive_mutex mutex;

    // todo:  refactor
    NetworkMessage msg;

    boost::asio::ip::tcp::socket socket;
	boost::asio::steady_timer readTimer;
    boost::asio::io_service& ioService;
    std::chrono::steady_clock::time_point sessionStartTime;
    std::shared_ptr<Dispatcher> dispatcher;
    std::shared_ptr<Protocol> protocol;

    bool sessionIsReady { false };
};
