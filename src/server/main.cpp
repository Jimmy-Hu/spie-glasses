//-----include header files, 引入標頭檔-----
#include "image_sink.hpp"														//	include "image_sink.hpp" header file, 引入"image_sink.hpp"標頭檔
#include "server.hpp"															//	include "server.hpp" header file, 引入"server.hpp"標頭檔
#include "session.hpp"															//	include "session.hpp" header file, 引入"session.hpp"標頭檔
#include "session_manager.hpp"													//	include "session_manager.hpp" header file, 引入"session_manager.hpp"標頭檔
// internal api
#include <api/Detect.hpp>														//	include <api/Detect.hpp> header file, 引入<api/Detect.hpp>標頭檔
// external api
#include <boost/asio.hpp>														//	引入<boost/asio.hpp>標頭檔
#include <boost/system/error_code.hpp>											//	引入<boost/system/error_code.hpp>標頭檔
#include <msgpack.hpp>															//	引入<msgpack.hpp>標頭檔
#include <json.hpp>																//	引入<json.hpp>標頭檔
// standard api
#include <cstdlib>																//	引入<cstdlib>標頭檔
#include <deque>																//	引入<deque>標頭檔
#include <iostream>																//	引入<iostream>標頭檔
#include <list>																	//	引入<list>標頭檔
#include <memory>																//	引入<memory>標頭檔
#include <set>																	//	引入<set>標頭檔
#include <map>																	//	引入<map>標頭檔
#include <utility>																//	引入<utility>標頭檔
#include <thread>																//	引入<thread>標頭檔
#include <algorithm>															//	引入<algorithm>標頭檔

int main(int argc, char* argv[])												//	main主程式
{
	try
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
        while (true) 
		{
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
