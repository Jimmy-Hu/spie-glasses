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
#include <nlohmann/json.hpp>
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
        , m_msgio(m_socket)
    {
        do_connect(endpoint_iterator);
        m_msgio.set_error_handler([this](auto const & ec){this->do_exit();});
        m_msgio.set_post_handler<void(std::vector<std::size_t>)>
            ("people_ids", [this](auto v){return this->on_people_ids_arrived(std::move(v));});
        m_msgio.set_post_handler<void(std::vector<std::string>)>
            ("people_names", [this](auto v){return this->on_people_names_arrived(std::move(v));});
    }

    void close() {
       m_ios.post([this]() {m_socket.close();});
    }

    void write(std::vector<std::string> const & v) {
        if (v.size() < 2) return;
        try {
            auto raw = json::to_msgpack(json::parse(v[1]));
            m_msgio.request_raw(v[0], raw, [](boost::system::error_code const & ec, msgpack::object_handle oh) {
                    if (!ec) std::cerr << "[info][response] object=" << oh.get() << std::endl;
                    else std::cerr << "[info][response] failed" << std::endl;
                });
            std::cerr << "[info][request] start" << std::endl;
        } catch(...) {
            std::cerr << "[info][request] wrong argument (json parsing error)" << std::endl;
        }
    }
private:
    void on_people_ids_arrived(std::vector<std::size_t> v) {
        std::cerr << "[info] people : " ;
        for (auto && x : v) {
            std::cerr << x << " ";
        }
        std::cerr << std::endl;
    }
    void on_people_names_arrived(std::vector<std::string> v) {
        std::cerr << "[info] people : " ;
        for (auto && x : v) {
            std::cerr << x << " ";
        }
        std::cerr << std::endl;
    }
    void login() {
        m_msgio.post("login", std::make_tuple("audio-client", 1));
    }
    void on_connected() {
        m_msgio.set_post_handler<void(std::vector<std::size_t>)>("people_ids", [this](auto v){ this->on_people_ids_arrived(std::move(v)); });
        login();
        m_msgio.async_dispatch();
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
    tcp::socket m_socket;
    api::message_io_object m_msgio;
};

#endif // API_SRC_IMAGE_CLIENT_CLIENT_HPP_INCLUDED
