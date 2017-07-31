#ifndef API_SRC_SIMPLE_SERVER_SERVER_HPP_INCLUDED
#define API_SRC_SIMPLE_SERVER_SERVER_HPP_INCLUDED

#include "session.hpp"
#include "session_manager.hpp"
// 
#include <api/asio_msgpack.hpp>
#include <api/msgpack_reader.hpp>
#include <api/msgpack_writer.hpp>
#include <api/cv_msgpack.hpp>
#include <api/image_sink.hpp>
#include <api/requester.hpp>
#include <api/responser.hpp>
#include <api/dispatcher.hpp>
#include <api/message_io_object.hpp>
#include <api/parser.hpp>
#include <api/pointer_compare.hpp>
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

class server
{
public:
    server(boost::asio::io_service& io_service,
                const boost::asio::ip::tcp::endpoint& endpoint)
        : m_acceptor(io_service, endpoint),
          m_socket(io_service)
        {
            do_accept();
        }
    void close() {
        m_acceptor.get_io_service().post(
            [this](){
                for (auto kv : m_mgr) {
                    kv.first->close();
                }
                m_mgr.clear();
                m_acceptor.close();
            });
    }
private:
    void do_accept() {
        m_acceptor.async_accept(
            m_socket,
            [this](boost::system::error_code ec) {
                if (!ec) {
                    auto sp = std::make_shared<session>(std::move(m_socket),m_mgr);
                    sp->start();
                    std::cerr << "[info] accept client-" << sp << std::endl;
                    do_accept();
                } else {
                    std::cerr << "[info] accept stop" << std::endl;
                }
            });
    }
    std::weak_ptr<session> m_wp; // current session
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket m_socket;
    session_manager m_mgr;
};

#endif // API_SRC_SIMPLE_SERVER_SERVER_HPP_INCLUDED
