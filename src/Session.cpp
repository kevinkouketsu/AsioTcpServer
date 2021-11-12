#include "Session.hpp"
#include <iostream>
#include "Protocol.hpp"

using namespace std::chrono_literals;

Session::Session(std::shared_ptr<Dispatcher> dispatcher, boost::asio::io_service& ioService)
    : ioService{ioService}
    , socket{ioService}
    , readTimer{ioService}
    , sessionStartTime{std::chrono::steady_clock::now()}
    , dispatcher{std::move(dispatcher)}
{}

boost::asio::ip::tcp::socket& Session::getSocket()
{
    return socket;
}

uint32_t Session::getIpAddress() const
{
    boost::asio::ip::tcp::endpoint endpoint;
    {
	    boost::system::error_code error;
        std::lock_guard<std::recursive_mutex> lockClass(mutex);
        endpoint = socket.remote_endpoint(error);
        if (error)
        {
            return 0;
        }
    }
	return htonl(endpoint.address().to_v4().to_ulong());
}

void Session::handleTimeout(const boost::system::error_code& error)
{
    if (error == boost::asio::error::operation_aborted) {
        return;
    }
    close(true);
}

void Session::setTimerTimeout(boost::asio::steady_timer& steadyTimer, std::chrono::seconds timeout)
{
    steadyTimer.expires_from_now(timeout);
    steadyTimer.async_wait(std::bind(&Session::handleTimeout, shared_from_this(), std::placeholders::_1));
}

void Session::read()
{
    std::lock_guard<std::recursive_mutex> lockGuard { mutex };

    setTimerTimeout(readTimer, 15s);
    if (sessionIsReady)
    {
        try
        {
            boost::asio::async_read(
                socket,
                boost::asio::buffer(msg.getBuffer(), NetworkMessage::SIZE_LENGTH),
                std::bind(&Session::parseHeader, shared_from_this(), std::placeholders::_1));
        }
        catch (const boost::system::system_error& e)
        {}
    }
    else
    {
        boost::asio::async_read(
            socket,
            boost::asio::buffer(msg.getBuffer(), 4),
            std::bind(&Session::parseHelloPacket, shared_from_this(), std::placeholders::_1));
    }
}

void Session::closeSocket()
{
    std::cout << "Closing socket" << std::endl;
    if (socket.is_open())
    {
        boost::system::error_code error;
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
        socket.close(error);
        dispatcher->addTask(createTask(std::bind(&Protocol::onClose, this->protocol)));
    }
}

void Session::close(bool forceClose)
{
    std::lock_guard<std::recursive_mutex> lockGuard { mutex };

    //  TODO: check message queue if something is missing
    if (forceClose)
        closeSocket();
}


void Session::parseHelloPacket(const boost::system::error_code& error)
{
    if (error)
    {
        close(ForceClose);
        return;
    }

    msg.setBufferPosition(0);
    if (msg.get<uint32_t>() == 0x1F11F311)
    {
        sessionIsReady = true;
        read();
    }
}

void Session::parseHeader(const boost::system::error_code& error)
{
    readTimer.cancel();

    if (error)
    {
        std::cout << error.message() << std::endl;
        // an error occured
        close(ForceClose);
        return;
    }

	auto size = msg.getLengthHeader() - NetworkMessage::SIZE_LENGTH;
    msg.setLength(size + NetworkMessage::SIZE_LENGTH);
    boost::asio::async_read(
        socket, boost::asio::buffer(msg.getBodyBuffer(), size),
        std::bind(&Session::parsePacket, shared_from_this(), std::placeholders::_1));
}

void Session::parsePacket(const boost::system::error_code& error)
{
    readTimer.cancel();
    if (error)
    {
        std::cout << error.message() << std::endl;
        close(ForceClose);
        return;
    }

    addTask(&Protocol::onRecvMessage, msg);
    read();
}

void Session::accept(std::shared_ptr<Protocol> protocol)
{
    this->protocol = std::move(protocol);

    addTask(&Protocol::onAccept);
}