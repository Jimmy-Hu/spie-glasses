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
#include <boost/asio/steady_timer.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <msgpack.hpp>
#include <json.hpp>
// standard api
#include <chrono>
#include <cstdlib>
#include <string>
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
        , m_start(false)
        , m_timer(io_service)
        {
            do_connect(endpoint_iterator);
            m_msgio.set_error_handler([this](auto const & ec){this->do_exit();});
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
    void post(std::vector<std::string> const & v) {
        if (v.size() < 2) return;
        try {
            auto raw = json::to_msgpack(json::parse(v[1]));
            m_msgio.post_raw(v[0], raw);
            std::cerr << "[info][post] start" << std::endl;
        } catch(...) {
            std::cerr << "[info][post] wrong argument (json parsing error)" << std::endl;
        }
    }
    void start_random_hint() {
        if (m_start) return;
        m_start = true;
        do_random_hint_names();
    }
    void stop_random_hint() {
        m_start = false;
    }
    void do_random_hint() {
        static char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        std::vector<std::string> v(std::rand()%5);
        auto gen_str = [](){
            std::string s;
            s.resize(std::rand()%16);
            std::generate(s.begin(),s.end(),[](){return chars[std::rand()%62];});
            return s;
        };
        std::generate(v.begin(),v.end(),gen_str);
        m_msgio.post("hint_names", std::make_tuple(v));
        m_timer.expires_from_now(std::chrono::seconds(1));
        m_timer.async_wait(
            [this](auto && ec){
                if (!ec && m_start)
                    this->do_random_hint();
                // Construct a timer with an absolute expiry time.
            });
    }
    void do_random_hint_names() {
        static char const *names[] = {"Lin", "Jimmy", "Hex", "Lee", "Frank", "Alice", "John", "Adamda"};
        bool name_used[8] = {false};
        std::vector<std::string> v(std::rand()%5);
        auto iter = v.begin();
        for (int j = 0;;j = (j + 1) % 8) {
            if (iter == v.end()) break;
            if (name_used[j]) continue;
            if (std::rand() % 2 == 0) 
                continue;
            *iter = names[j];
            name_used[j] = true;
            ++iter;
        }
        m_msgio.post("hint_names", std::make_tuple(v));
        m_timer.expires_from_now(std::chrono::seconds(1));
        m_timer.async_wait(
            [this](auto && ec){
                if (!ec && m_start)
                    this->do_random_hint_names();
                // Construct a timer with an absolute expiry time.
            });
    }
    private:
            void login() {
            m_msgio.post("login", std::make_tuple("hint-client", 1));
        }
        void on_connected() {
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
        // std::uniboost::asio::io_service::work m_work;
        tcp::socket m_socket;
        api::message_io_object m_msgio;
        bool m_start;
        boost::asio::steady_timer m_timer;
    };

#endif // API_SRC_IMAGE_CLIENT_CLIENT_HPP_INCLUDED
