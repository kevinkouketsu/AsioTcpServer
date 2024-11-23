#pragma once

#include "Dispatcher.hpp"
#include "NetworkMessage.hpp"
#include "SessionFactory.hpp"
#include <boost/asio.hpp>
#include <list>

class Dispatcher;
class BufferWriter;
class Session;

namespace {

    template<typename SessionType>
    class DefaultSessionFactory : public SessionFactory<SessionType>
    {
    public:
        std::shared_ptr<SessionType> create(boost::asio::io_service& ioService)
        {
            return std::make_shared<SessionType>(ioService);
        }
    };

}

enum class Transport
{
    // The two first bytes represents the size of message
    FramedTcp,
};

class Session : public std::enable_shared_from_this<Session>
{
public:
    static constexpr bool ForceClose = true;

    // The SessionHandler lifetime must be the same as the Session
    Session(boost::asio::io_service& ioService);
    Session(boost::asio::io_service& ioService, Transport transport);

    void connect(const std::string& ipAddress, uint16_t port)
    {
        socket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address(boost::asio::ip::address_v4::from_string(ipAddress)), port));
        read();
    }

    virtual void onReceiveMessage(const NetworkMessage& message)
    {

    }

    virtual void onSendMessage(std::shared_ptr<BufferWriter>& message)
    {

    }

    boost::asio::ip::tcp::socket& getSocket()
    {
        return this->socket;
    }

    uint32_t getIpAddress() const;

    void read();
    void send(std::shared_ptr<BufferWriter> message);

    void close(bool force = false);
    void setTimeout(std::chrono::microseconds timeoutInMicroseconds);

private:
    void parseHeader(const boost::system::error_code& error);
    void parsePacket(const boost::system::error_code& error);
    void closeSocket();
    void handleTimeout(const boost::system::error_code& error);
    void setTimerTimeout(boost::asio::steady_timer& steadyTimer, std::chrono::microseconds timeout);
    void internalSend(std::shared_ptr<BufferWriter> message);
    void onWriteOperation(const boost::system::error_code& error);

    void setTransport(Transport transport)
    {
        this->transport = transport;
    }

private:
    mutable std::recursive_mutex mutex;

    NetworkMessage msg;

    bool closed { false };

    boost::asio::ip::tcp::socket socket;
	boost::asio::steady_timer readTimer;
    boost::asio::io_service& ioService;
    std::chrono::steady_clock::time_point sessionStartTime;
    std::shared_ptr<Dispatcher> dispatcher;
    std::chrono::microseconds timeoutInMicroseconds;

    std::list<std::shared_ptr<BufferWriter>> pendingMessagesQueue;

    Transport transport;
    size_t maximumPendingMessages { 0 };
};
