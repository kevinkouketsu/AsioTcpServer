#include "Server.hpp"
#include <iostream>
#include <boost/asio.hpp>
#include "Dispatcher.hpp"
#include "Scheduler.hpp"
#include "Service.hpp"
#include "Protocol.hpp"
#include "NetworkMessage.hpp"
#include "Client.hpp"
#include "BufferWriter.hpp"
#include "Session.hpp"
#include "ILogger.hpp"

static uint8_t KeyTable[] =
{
    0x14, 0x17, 0x47, 0x67, 0x7A, 0x09, 0x21, 0x0D, 0x5B, 0x5B, 0x15, 0x0D, 0x17, 0x11, 0x21, 0x0C, 0x1F, 0x03, 0x21,
    0x21, 0x17, 0x0D, 0x1D, 0x0D, 0x16, 0x1F, 0x03, 0x1F, 0x71, 0x6D, 0x15, 0x0D, 0x15, 0x0D, 0x15, 0x13, 0x17, 0x2C,
    0x15, 0x43, 0x1D, 0x72, 0x17, 0x29, 0x1F, 0x09, 0x15, 0x16, 0x47, 0x0D, 0x67, 0x6D, 0x79, 0x0D, 0x67, 0x0D, 0x15,
    0x09, 0x15, 0x0D, 0x1F, 0x71, 0x17, 0x0E, 0x33, 0x17, 0x05, 0x09, 0x6F, 0x73, 0x5B, 0x13, 0x33, 0x32, 0x3E, 0x1E,
    0x24, 0x0D, 0x6E, 0x0E, 0x15, 0x0A, 0x15, 0x3F, 0x5D, 0x0D, 0x17, 0x35, 0x17, 0x0D, 0x71, 0x0D, 0x18, 0x0D, 0x25,
    0x21, 0x33, 0x0D, 0x17, 0x0C, 0x1D, 0x0A, 0x15, 0x17, 0x27, 0x0C, 0x15, 0x0D, 0x3C, 0x10, 0x4B, 0x09, 0x14, 0x2B,
    0x6B, 0x35, 0x67, 0x1F, 0x15, 0x1F, 0x15, 0x0E, 0x15, 0x10, 0x15, 0x28, 0x05, 0x2D, 0x33, 0x2A, 0x1D, 0x29, 0x17,
    0x0C, 0x15, 0x0D, 0x14, 0x0D, 0x15, 0x0E, 0x77, 0x27, 0x1D, 0x1F, 0x15, 0x0B, 0x7A, 0x0D, 0x3D, 0x10, 0x3D, 0x0D,
    0x47, 0x3F, 0x1D, 0x0D, 0x79, 0x4D, 0x15, 0x0D, 0x17, 0x47, 0x33, 0x0D, 0x77, 0x47, 0x33, 0x1C, 0x17, 0x0E, 0x15,
    0x35, 0x0D, 0x06, 0x45, 0x49, 0x1D, 0x7F, 0x33, 0x0D, 0x17, 0x2B, 0x15, 0x1C, 0x71, 0x31, 0x1D, 0x0F, 0x17, 0x0D,
    0x14, 0x0A, 0x14, 0x0B, 0x71, 0x16, 0x78, 0x7F, 0x61, 0x09, 0x15, 0x29, 0x63, 0x25, 0x53, 0x57, 0x29, 0x0D, 0x77,
    0x1C, 0x47, 0x0C, 0x33, 0x0D, 0x15, 0x0D, 0x5B, 0x09, 0x31, 0x35, 0x17, 0x0D, 0x29, 0x0D, 0x1D, 0x0D, 0x25, 0x21,
    0x33, 0x0D, 0x17, 0x0C, 0x15, 0x0A, 0x15, 0x3F, 0x5D, 0x0D, 0x17, 0x0D, 0x79, 0x4D, 0x15, 0x0D, 0x25, 0x09, 0x15,
    0x0D, 0x51, 0x0B, 0x7A, 0x0D, 0x47, 0x0D, 0x15, 0x0D, 0x15, 0x0D, 0x1D, 0x0D, 0x79, 0x03, 0x15, 0x09, 0x15, 0x0D,
    0x67, 0x0D, 0x15, 0x71, 0x49, 0x71, 0x1F, 0x75, 0x15, 0x16, 0x3D, 0x0D, 0x67, 0x6D, 0x33, 0x1E, 0x76, 0x0D, 0x6E,
    0x0E, 0x3E, 0x1E, 0x1F, 0x71, 0x19, 0x0E, 0x33, 0x0D, 0x05, 0x09, 0x33, 0x71, 0x5B, 0x13, 0x1C, 0x1F, 0x15, 0x0B,
    0x15, 0x0E, 0x1F, 0x10, 0x15, 0x28, 0x05, 0x0A, 0x15, 0x2A, 0x1D, 0x71, 0x1F, 0x0C, 0x19, 0x1C, 0x15, 0x1B, 0x33,
    0x79, 0x17, 0x0B, 0x33, 0x1C, 0x2F, 0x47, 0x31, 0x0A, 0x18, 0x0E, 0x1F, 0x35, 0x0D, 0x10, 0x47, 0x49, 0x28, 0x4F,
    0x5B, 0x29, 0x15, 0x35, 0x21, 0x10, 0x17, 0x11, 0x17, 0x0C, 0x1F, 0x03, 0x21, 0x21, 0x14, 0x17, 0x47, 0x67, 0x16,
    0x09, 0x71, 0x6D, 0x15, 0x0A, 0x03, 0x2B, 0x15, 0x0D, 0x1D, 0x13, 0x17, 0x2C, 0x15, 0x43, 0x17, 0x0D, 0x15, 0x1F,
    0x17, 0x0D, 0x1D, 0x0D, 0x06, 0x0E, 0x17, 0x0D, 0x18, 0x29, 0x19, 0x05, 0x61, 0x6D, 0x15, 0x0D, 0x1B, 0x53, 0x7A,
    0x0A, 0x67, 0x40, 0x1D, 0x0D, 0x17, 0x35, 0x17, 0x0C, 0x03, 0x0E, 0x0D, 0x16, 0x17, 0x33, 0x15, 0x20, 0x67, 0x6F,
    0x7D, 0x35, 0x71, 0x0A, 0x15, 0x33, 0x7A, 0x0E, 0x15, 0x28, 0x3D, 0x09, 0x16, 0x0D, 0x15, 0x0D, 0x67, 0x0D, 0x71,
    0x0A, 0x05, 0x0D, 0x15, 0x40, 0x3B, 0x47, 0x71, 0x0A, 0x17, 0x09, 0x14, 0x0D, 0x03, 0x03, 0x17, 0x0D, 0x33, 0x0D,
    0x79, 0x0D, 0x15, 0x0E, 0x12, 0x0D, 0x6D, 0x3D, 0x17, 0x09, 0x77, 0x09, 0x3D, 0x0C, 0x33, 0x6A, 0x17, 0x1D, 0x1D,
    0x0B, 0x77, 0x09, 0x2B, 0x0D, 0x67, 0x1F, 0x15, 0x0D, 0x1D, 0x44, 0x1F, 0x0D, 0x3D, 0x17, 0x79, 0x0C, 0x15, 0x10,
    0x15, 0x09, 0x1A, 0x53, 0x77, 0x35, 0x78, 0x7B, 0x1D, 0x04, 0x20, 0x03, 0x43, 0x27, 0x1D, 0x47, 0x31, 0x29
};

struct PacketHeader
{
    PacketHeader() = default;
    PacketHeader(uint16_t size, uint16_t type)
        : Size(size)
        , Type(type)
        , KeyWord{ 0 }
        , CheckSum{ 0 }
        , ID{ 0 }
        , Tick{ 0 }
    {}
    uint16_t Size;
    uint8_t KeyWord;
    uint8_t CheckSum;
    uint16_t Type;
    uint16_t ID;
    uint32_t Tick;
};

bool decryptMessage(uint8_t* PacketBuffer, uint16_t size)
{
    PacketHeader* Header = (PacketHeader*)PacketBuffer;

    int32_t KeyIncrement = KeyTable[Header->KeyWord * 2];
    for (int16_t i = 4; i < size; i++, KeyIncrement++)
    {
        int32_t KeyResult = KeyTable[((KeyIncrement & 0x800000FF) * 2) + 1];

        switch (i & 3)
        {
        case 00:
            PacketBuffer[i] -= ((KeyResult & 255) << 1);
            break;
        case 01:
            PacketBuffer[i] += ((KeyResult & 255) >> 3);
            break;
        case 02:
            PacketBuffer[i] -= ((KeyResult & 255) << 2);
            break;
        case 03:
            PacketBuffer[i] += ((KeyResult & 255) >> 5);
            break;
        }
    }

    return true;
}

void PacketEncrypt(uint8_t* pBuffer, const uint8_t* data, uint32_t packetSize)
{
	PacketHeader* pHeader = (PacketHeader*)pBuffer;
	uint32_t checkSum[2] = { 0, 0 };

	uint8_t hashKey = KeyTable[15];
	pHeader->Size = (int16_t)packetSize;
	pHeader->KeyWord = hashKey;
	pHeader->Tick = std::chrono::steady_clock::now().time_since_epoch().count();

	uint32_t keyIncrement = KeyTable[(hashKey & 255) * 2];

	for (uint32_t i = 4; i < packetSize; i++, keyIncrement++)
	{
		uint32_t keyResult = KeyTable[(((keyIncrement & 255) & 0x800000FF) * 2) + 1];

		switch (i & 3)
		{
		case 00:
		{
				   pBuffer[i] = data[i] + ((keyResult & 255) << 1);
				   break;
		}
		case 01:
		{
				   pBuffer[i] = data[i] - ((keyResult & 255) >> 3);
				   break;
		}
		case 02:
		{
				   pBuffer[i] = data[i] + ((keyResult & 255) << 2);
				   break;
		}
		case 03:
		{
				   pBuffer[i] = data[i] - ((keyResult & 255) >> 5);
				   break;
		}
		}

		checkSum[0] += data[i];
		checkSum[1] += pBuffer[i];
	}

	pHeader->CheckSum = ((checkSum[1] & 255) - (checkSum[0] & 255)) & 255;
}

class CustomSession : public Session
{
public:
    CustomSession(std::shared_ptr<Dispatcher> dispatcher, boost::asio::io_service& ioService, int a)
        : Session(dispatcher, ioService)
    {}

};


class CustomSessionFactory : public SessionFactory<CustomSession>
{
public:
    std::shared_ptr<CustomSession> create(std::shared_ptr<Dispatcher> dispatcher, boost::asio::io_service& ioService) override
    {
        return std::make_shared<CustomSession>(dispatcher, ioService, 1);
    }
};

class Logger : public network::ILogger
{
public:
    virtual ~Logger() = default;
    void debug(const std::string& fileName, uint32_t line, const std::string& method, const std::string& message)
    {
        std::cout << "[DEBUG][" << fileName << "][" << line << "][" << method << "][" << message << "]" << std::endl;
    }
    void error(const std::string& fileName, uint32_t line, const std::string& method, const std::string& message)
    {
        std::cout << "[ERROR][" << fileName << "][" << line << "][" << method << "][" << message << "]" << std::endl;
    }
    void info(const std::string& fileName, uint32_t line, const std::string& method, const std::string& message)
    {
        std::cout << "[INFO][" << fileName << "][" << line << "][" << method << "][" << message << "]" << std::endl;
    }
};

int main(int argc, char* argv[])
{
    class ProtocolTest : public Protocol
    {
    public:
        ProtocolTest(std::shared_ptr<Dispatcher> dispatcher, std::shared_ptr<Scheduler> scheduler, std::shared_ptr<CustomSession> session)
            : session{std::move(session)}
        {
        }
        void start() override
        {

        }
        void onAccept() override
        {
            std::cout << "onAccept=" << this << std::endl;
        }
        void onClose() override
        {
            std::cout << "onClose=" << this << std::endl;
        }
        void onRecvMessage(NetworkMessage& msg) override
        {
            decryptMessage(msg.getBuffer(), msg.getLengthHeader());
            msg += 2;

            std::cout << "PacketId " << std::hex << msg.get<uint16_t>() << std::endl;

            BufferWriter writer{ 140 };
            writer.set<uint16_t>(140);
            writer.set<uint16_t>(0);
            writer.set<uint16_t>(0x101);
            writer += 6;

            writer << 'A' << 'B' << 'C' << '\0';
            session->send(std::make_shared<BufferWriter>(writer));
        }
        void onSendMessage(const std::shared_ptr<BufferWriter>& message)
        {
            auto oldData = message->getBuffer();
            PacketEncrypt(message->getBuffer().data(), oldData.data(), message->getSize());
            std::cout << "trying to send a new message" << std::endl;
        }

        std::shared_ptr<CustomSession> session;
    };

    class ProtocolClient : public Protocol
    {
    public:
        ProtocolClient(std::shared_ptr<Dispatcher> dispatcher, std::shared_ptr<Scheduler> scheduler, std::shared_ptr<CustomSession> session)
        {
        }
        void start() override
        {

        }
        void onAccept() override
        {
            std::cout << "onConnected into the client=" << this << std::endl;
        }
        void onClose() override
        {
            std::cout << "Client closed=" << this << std::endl;
        }
        void onRecvMessage(NetworkMessage& msg) override
        {
            decryptMessage(msg.getBuffer(), msg.getLengthHeader());
            msg += 2;

            std::cout << "PacketId " << std::hex << msg.get<uint16_t>() << std::endl;
        }
        void onSendMessage(const std::shared_ptr<BufferWriter>& message)
        {
        }
    };
    try
    {
        network::ILogger::setLogger(std::make_shared<Logger>());
        auto dispatcher = std::make_shared<Dispatcher>();
        auto scheduler = std::make_shared<Scheduler>(dispatcher);
        dispatcher->addTask(createTask([](){
            std::cout << "From another thread " << std::this_thread::get_id() << std::endl;
        }));

        std::cout << "From main thread " << std::this_thread::get_id() << std::endl;
        auto eventId = scheduler->addEvent(createTask<SchedulerTask>(std::chrono::seconds(2), [dispatcher]()
        {
            std::cout << "Calling the scheduled task..." << std::endl;
        }));

        auto sessionFactory = std::make_shared<CustomSessionFactory>();
        auto services = std::make_shared<Services<CustomSession>>(dispatcher, scheduler, sessionFactory);
        services->add<ProtocolTest>(8174, "");

        auto client = std::make_shared<ClientService<CustomSession>>(dispatcher, scheduler, services->getIoService(), std::make_shared<ProtocolFactory<ProtocolClient, CustomSession>>(), sessionFactory);
        std::string ipAddress = std::string{"127.0.0.1"};
        client->open(ipAddress, 8174);

        services->run();
        dispatcher->join();
        scheduler->shutdown();
        scheduler->join();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
