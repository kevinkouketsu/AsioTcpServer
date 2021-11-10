#include "Session.hpp"
#include <iostream>
#include "Protocol.hpp"

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
    close(true);
}

void Session::read()
{
    std::lock_guard<std::recursive_mutex> lockGuard { mutex };
    try
    {
		readTimer.expires_from_now(std::chrono::seconds(15));
		readTimer.async_wait(std::bind(&Session::handleTimeout, shared_from_this(), std::placeholders::_1));

		boost::asio::async_read(
            socket,
            boost::asio::buffer(msg.getBuffer(), 4),
            std::bind(&Session::parseHeader, shared_from_this(), std::placeholders::_1));
    }
    catch (const boost::system::system_error& e)
    {}
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

void Session::parseHeader(const boost::system::error_code& error)
{
    if (error)
    {
        // an error occured
        close(ForceClose);
        return;
    }

	auto size = msg.get<int>();
    std::cout << "Packet of " << size << " arrived " << std::endl;

    msg.setLength(size + NetworkMessage::HEADER_LENGTH);
    boost::asio::async_read(
        socket, boost::asio::buffer(msg.getBodyBuffer(), size),
        std::bind(&Session::parsePacket, shared_from_this(), std::placeholders::_1));
}

void Session::parsePacket(const boost::system::error_code& error)
{
    if (error)
    {
        // an error occured
        close(ForceClose);
        return;
    }

    readTimer.cancel();
}


void Session::accept(std::shared_ptr<Protocol> protocol)
{
    this->protocol = std::move(protocol);

    addTask(&Protocol::onAccept);
    addTask(Protocol::onAccept);
}