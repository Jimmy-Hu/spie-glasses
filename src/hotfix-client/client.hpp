#ifndef API_SRC_IMAGE_CLIENT_CLIENT_HPP_INCLUDED
#define API_SRC_IMAGE_CLIENT_CLIENT_HPP_INCLUDED

// internal api
#include <api/asio_msgpack.hpp>
#include <api/message_io_object.hpp>
#include <api/msgpack_reader.hpp>
#include <api/msgpack_writer.hpp>
#include <api/cv_msgpack.hpp>
#include <api/image_sink.hpp>
#include <api/requester.hpp>
#include <api/responser.hpp>
#include <api/dispatcher.hpp>
#include <api/parser.hpp>
#include <api/pointer_compare.hpp>
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

using boost::asio::ip::tcp;
using json = nlohmann::json ;

class client
{
public:
    using command_type = std::vector<std::string>;
    client(boost::asio::io_service& io_service,
                tcp::resolver::iterator endpoint_iterator)
        : m_ios(io_service)
        , m_socket(io_service)
        , m_w(m_socket)
        , m_work(io_service)
        , m_msgio(m_socket)
    {
        do_connect(endpoint_iterator);
        m_msgio.set_error_handler([this](auto const & ec){this->do_exit();});
    }

    void close() {
       m_ios.post([this]() {m_socket.close();});
    }
private:
    void on_connected() {
        m_msgio.async_dispatch();
        m_msgio.post("asdf",std::vector<std::string>{"hello", "world"});
        m_msgio.post("asdf",std::vector<std::string>{"hello", "world"});
        m_msgio.post("asdf",std::vector<std::string>{"hello", "world"});
    }
    void do_write() {
        auto sbuf = std::make_shared<msgpack::sbuffer>();
        std::vector<std::string> v {"hello","world"};
        msgpack::pack(*sbuf, v);
        msgpack::pack(*sbuf, v);
        m_w.async_write(*sbuf,[sbuf](auto && ec) { std::cerr << "write done" << std::endl; });
    }
    void do_connect(tcp::resolver::iterator endpoint_iterator)
        {
            boost::asio::async_connect(
                m_socket, endpoint_iterator,
                [this](boost::system::error_code const & ec, tcp::resolver::iterator)
                {
                    if (!ec) {
                        this->on_connected();
                    }
                    else {
                        this->do_exit();
                    }
                });
        }
    void do_exit() {
        m_socket.close();
    }
private:
    boost::asio::io_service& m_ios;
    boost::asio::io_service::work m_work;
    tcp::socket m_socket;
    api::msgpack_writer m_w;
    api::message_io_object m_msgio;
};

#endif // API_SRC_IMAGE_CLIENT_CLIENT_HPP_INCLUDED
