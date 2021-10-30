#include "Server.hpp"
#include <iostream>
#include <asio.hpp>

awaitable<void> listener(tcp::acceptor acceptor)
{
    co_await acceptor.async_accept(use_awaitable);
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: chat_server <port> [<port> ...]\n";
            return 1;
        }

        asio::io_context io_context(1);

        for (int i = 1; i < argc; ++i)
        {
            unsigned short port = std::atoi(argv[i]);
            co_spawn(io_context,
                     [&io_context, port]
                         {
                             return listener(tcp::acceptor(io_context, {tcp::v4(), port}));
                         },
                     detached);
        }

        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto){ io_context.stop(); });

        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
