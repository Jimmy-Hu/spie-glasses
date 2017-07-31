//
// chat_server.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>
#include <api/asio_msgpack.hpp>

using boost::asio::ip::tcp;
//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
    boost::asio::io_service ios;
    boost::system::error_code ec;
    boost::asio::streambuf buf;
    std::istream istm(&buf);
    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: chat_server <port>\n";
            return 1;
        }
        std::vector<char> v(512);
        tcp::endpoint ep(tcp::v4(), std::atoi(argv[1]));
        tcp::acceptor acc(ios,ep);
        tcp::socket soc(ios);
        acc.accept(soc,ec);
        if (ec) return 1;
        std::cerr << "accept" << std::endl;
        while (true) {
            auto n = soc.read_some(boost::asio::buffer(v.data(),v.size()), ec);
            if (ec) return 1;
            buf.commit(n);
            if (n == 0) continue;
            std::cout.write(v.data(), n);
            std::cout.flush();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
