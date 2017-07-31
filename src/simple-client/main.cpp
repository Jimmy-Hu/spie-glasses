#include "client.hpp"

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: client <host> <port>\n";
            return 1;
        }

        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        auto endpoint_iterator = resolver.resolve({ argv[1], argv[2] });
        client c(io_service, endpoint_iterator);

        bool exit = false;
        std::thread t(
            [&io_service](){
                io_service.run();
                std::cerr << "[thread] ending" << std::endl;
            });
        std::string s;
        bool continued = false;
        api::command_parser cp;
        while (true) {
            if (!std::getline(std::cin,s))
                break;
            if (s == ":exit")
                break;
            if (exit)
                break;
            cp.parse(s.begin(),s.end());
            if (cp.is_complete()) {
                auto v = cp.get();
                c.write(std::move(v));
                cp.clear();
                continued = false;
            }
            else {
                continued = true;
            }
        }
        c.close();
        t.join();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
