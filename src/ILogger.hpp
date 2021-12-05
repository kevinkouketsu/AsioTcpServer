#pragma once

#include <string>
#include <memory>
#include <sstream>

namespace network {

    class ILogger
    {
    public:
        virtual ~ILogger() = default;
        virtual void debug(const std::string& fileName, uint32_t line, const std::string& method, const std::string& message) = 0;
        virtual void error(const std::string& fileName, uint32_t line, const std::string& method, const std::string& message) = 0;
        virtual void info(const std::string& fileName, uint32_t line, const std::string& method, const std::string& message) = 0;

        static std::shared_ptr<ILogger> getInstance()
        {
            return logger;
        }
        static void setLogger(std::shared_ptr<ILogger> logger)
        {
            ILogger::logger = std::move(logger);
        }

    private:
        inline static std::shared_ptr<ILogger> logger;
    };

}

#define NETWORK_LOG_DEBUG_STR(msg) if (::network::ILogger::getInstance()) ::network::ILogger::getInstance()->debug(__FILE__, __LINE__, __FUNCTION__, msg);
#define NETWORK_LOG_ERROR_STR(msg) if (::network::ILogger::getInstance()) ::network::ILogger::getInstance()->error(__FILE__, __LINE__, __FUNCTION__, msg);
#define NETWORK_LOG_INFO_STR(msg) if (::network::ILogger::getInstance()) ::network::ILogger::getInstance()->info(__FILE__, __LINE__, __FUNCTION__, msg);

#define NETWORK_LOG_DEBUG(msg) \
do {\
    std::stringstream ss;\
    ss << msg; \
    auto message = ss.str();\
    NETWORK_LOG_DEBUG_STR(message);\
} while(0)

#define NETWORK_LOG_INFO(msg) \
do {\
    std::stringstream ss;\
    ss << msg; \
    auto message = ss.str();\
    NETWORK_LOG_INFO_STR(message);\
} while(0)

#define NETWORK_LOG_ERROR(msg) \
do {\
    std::stringstream ss;\
    ss << msg; \
    auto message = ss.str();\
    NETWORK_LOG_ERROR_STR(message);\
} while(0)

