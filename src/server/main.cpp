#include "image_sink.hpp"
#include "server.hpp"
#include "session.hpp"
#include "session_manager.hpp"
// internal api
#include <api/Detect.hpp>
// external api
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <msgpack.hpp>
#include <json.hpp>
// standard api
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <map>
#include <utility>
#include <thread>
#include <algorithm>

int main(int argc, char* argv[])
{    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: server <port>\n";
            return 1;
        }
        api::Detect d;
        std::cerr << "[info] loading database" << std::endl;
        d.open();
        if (!d.is_opened()) {
            std::cerr << "[info] error loading database" << std::endl;
            return 0;
        }
        std::cerr << "[info] loading database done" << std::endl;
        boost::asio::io_service io_service;
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), std::atoi(argv[1]));
        server s(io_service,endpoint,d);
        std::thread t([&]() { io_service.run();});
        image_sink().open();
        std::string str;
        api::command_parser cp;
        bool continued = false;
        while (true) {
            if (!std::getline(std::cin,str))
                break;
            if (str == ":exit") {
                break;
            }
            cp.parse(str.begin(),str.end());
            if (cp.is_complete()) {
                auto v = cp.get();
                s.write(std::move(v));
                cp.clear();
                continued = false;
            }
            else {
                continued = true;
            }
        }
        s.close();
        std::cerr << "[info] closing server" << std::endl;
        image_sink().close();
        t.join();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
