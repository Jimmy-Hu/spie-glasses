#include "client.hpp"

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 3)
        {
            std::cerr << "Usage: client <host> <port> [camera index]\n";
            return 1;
        }
		cv::VideoCapture cap;
        if (argc == 4)
        {
            cap.open(argv[3] - (char)'0');
        }
        else
        {
            cap.open(0);
        }
        if (!cap.isOpened()) {
            std::cerr << "[info] camera not found." << std::endl;
            return 0;
        }
        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        auto endpoint_iterator = resolver.resolve({ argv[1], argv[2] });
        client c(io_service, endpoint_iterator,cap);

        std::thread t(
            [&io_service](){
                io_service.run();
                std::cerr << "[thread] ending" << std::endl;
            });
        std::string s;
        bool continued = false;
        bool exit = false;
        api::command_parser cp;
        while (true) {
            if (!std::getline(std::cin,s))
                break;
            if (s == ":exit")
                break;
            if (s == ":raw") {
                c.set_mode(0);
                std::cerr << "[info] activate raw mode" << std::endl;
            }
            else if (s == ":gz") {
                c.set_mode(1);
                std::cerr << "[info] activate gz mode" << std::endl;
            }
            else if (s == ":lz4") {
                c.set_mode(2);
                std::cerr << "[info] activate lz4 mode" << std::endl;
            }
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
        if (!exit) t.join();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
