#include "Session.hpp"
#include <iostream>
#include "Protocol.hpp"
#include "BufferWriter.hpp"
#include "ILogger.hpp"

using namespace std::chrono_literals;

Session::Session(boost::asio::io_service& ioService)
    : Session { ioService, Transport::FramedTcp }
{}

Session::Session(boost::asio::io_service& ioService, Transport transport)
    : ioService{ioService}
    , socket{ioService}
    , readTimer{ioService}
    , sessionStartTime{std::chrono::steady_clock::now()}
    , transport{transport}
    , timeoutInMicroseconds{std::chrono::seconds::zero()}
{}

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
    NETWORK_LOG_INFO("Something just timed out. Closing the connection");
    close(true);
}

void Session::setTimerTimeout(boost::asio::steady_timer& steadyTimer, std::chrono::microseconds timeout)
{
    if (timeout != std::chrono::microseconds::zero())
    {
        steadyTimer.expires_from_now(timeout);
        steadyTimer.async_wait(std::bind(&Session::handleTimeout, this->shared_from_this(), std::placeholders::_1));
    }
}

void Session::read()
{
    std::lock_guard<std::recursive_mutex> lockGuard { mutex };

    setTimerTimeout(readTimer, timeoutInMicroseconds);
    if (transport == Transport::FramedTcp)
    {
        try
        {
            boost::asio::async_read(
                socket,
                boost::asio::buffer(msg.getBuffer(), NetworkMessage::SIZE_LENGTH), 
                std::bind(&Session::parseHeader, this->shared_from_this(), std::placeholders::_1));
        }
        catch (const boost::system::system_error& error)
        {
            NETWORK_LOG_ERROR("Error " << error.code() << " while async_read. Message: " << error.what());
        }
    }
    else
    {
        throw std::runtime_error{"Not implemented"};
    }
}

void Session::closeSocket()
{
    if (socket.is_open())
    {
        boost::system::error_code error;
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
        socket.close(error);
    }
}

void Session::close(bool forceClose)
{
    std::lock_guard<std::recursive_mutex> lockGuard { mutex };
    if (closed)
        return;

    closed = true;
	if (pendingMessagesQueue.empty() || forceClose)
    {
		closeSocket();
	}
}


void Session::parseHeader(const boost::system::error_code& error)
{
    if (error)
    {
        NETWORK_LOG_ERROR("Error while trying to parseHeader: " << error.message() << ". Forcing close the connection");
        close(ForceClose);
        return;
    }

	auto size = msg.getLengthHeader() - NetworkMessage::SIZE_LENGTH;
    msg.setLength(size + NetworkMessage::SIZE_LENGTH);
    boost::asio::async_read(
        socket, boost::asio::buffer(msg.getBodyBuffer(), size),
        std::bind(&Session::parsePacket, this->shared_from_this(), std::placeholders::_1));
}

void Session::parsePacket(const boost::system::error_code& error)
{
    readTimer.cancel();
    if (error)
    {
        NETWORK_LOG_ERROR("Error while trying to parsePacket: " << error.message() << ". Forcing close the connection");
        close(ForceClose);
        return;
    }

    onReceiveMessage(msg);
    read();
}

void Session::send(std::shared_ptr<BufferWriter> message)
{
    bool isSleeping { false };
    {
        std::lock_guard<std::recursive_mutex> lockGuard { mutex };
        isSleeping = pendingMessagesQueue.empty();
        pendingMessagesQueue.push_back(message);
        auto oldMax = maximumPendingMessages;
        maximumPendingMessages = std::max(maximumPendingMessages, pendingMessagesQueue.size());
        if (oldMax != maximumPendingMessages)
        {
            NETWORK_LOG_DEBUG("Maximum pending messages exceeded: " << maximumPendingMessages);
        }
    }

    if (isSleeping)
    {
        internalSend(message);
    }
}

void Session::internalSend(std::shared_ptr<BufferWriter> message)
{
	this->onSendMessage(message);
	try {
		boost::asio::async_write(
            socket,
            boost::asio::buffer(
                message->getBuffer(),
                message->getSize()
            ),
            std::bind(
                &Session::onWriteOperation,
                this->shared_from_this(),
                std::placeholders::_1
            )
        );
	}
    catch (boost::system::system_error& e)
    {
		NETWORK_LOG_ERROR("Fail to send packet " << e.what() << ". Forced to close the connection");
		close(ForceClose);
	}
}

void Session::onWriteOperation(const boost::system::error_code& error)
{
	std::lock_guard<std::recursive_mutex> lockGuard { mutex };
	pendingMessagesQueue.pop_front();

	if (error)
    {
		pendingMessagesQueue.clear();
        NETWORK_LOG_ERROR("Fail to send a message " << error.message());
		close(ForceClose);
		return;
	}

	if (!pendingMessagesQueue.empty())
    {
		internalSend(pendingMessagesQueue.front());
	}
    else if (closed)
    {
		closeSocket();
        NETWORK_LOG_ERROR("Finished to send all queued messages. Closing the connection");
	}
}

void Session::setTimeout(std::chrono::microseconds timeoutInMicroseconds)
{
    this->timeoutInMicroseconds = timeoutInMicroseconds;
}
