#ifndef API_SRC_SIMPLE_SERVER_SESSION_HPP_INCLUDED
#define API_SRC_SIMPLE_SERVER_SESSION_HPP_INCLUDED

// target local api
#include "session_manager.hpp"
#include "image_sink.hpp"
// internal api
#include <api/logging.hpp>
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
#include <api/message_io_object.hpp>
// external api
#include <msgpack.hpp>
#include <boost/asio.hpp>
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

using json = nlohmann::json;
class session
    : public std::enable_shared_from_this<session>
{
public:
    using command_type = msgpack::object_handle;
    session(boost::asio::ip::tcp::socket socket, session_manager & mgr, api::Detect & d)
        : m_stop(true)
        , m_socket(std::move(socket))
        , m_sessions(mgr)
        , m_msgio(m_socket)
        , m_d(d)
        {}

    void start() {
        auto sp = this->shared_from_this();
        std::cerr << "[info] client #" << sp << " start." << std::endl;
        m_sessions.insert(sp);
        m_msgio.set_error_handler([this](auto const & ec){this->do_exit();});
        m_msgio.set_request_handler<int(int,int)>("add",std::plus<int>{});
        m_msgio.set_request_handler<int(int,int)>("sub",std::minus<int>{});
        m_msgio.set_request_handler<int(int,int)>("div",std::divides<int>{});
        m_msgio.set_request_handler<int(int,int)>("mul",std::multiplies<int>{});
        m_msgio.async_dispatch();
    }
    void close() {
        m_socket.get_io_service().post([this](){ m_socket.close();});
    }
    void write(std::vector<std::string> const & v) {
        auto sp = this->shared_from_this();
        if (!sp) return;
        if (v.size() < 2) return;
        try {
            auto raw = json::to_msgpack(json::parse(v[1]));
            m_msgio.request_raw(v[0], raw, [sp](boost::system::error_code const & ec, msgpack::object_handle oh) {
                    if (!ec) std::cerr << "[info][response][#" << sp << "] object=" << oh.get() << std::endl;
                    else std::cerr << "[info][response][#" << sp << "] failed" << std::endl;
                });
            std::cerr << "[info][request][#" << sp << "]" << std::endl;
        } catch(...) {
            std::cerr << "[info] wrong argument (json parsing error)" << std::endl;
        }
    }
    void show_video() {
        using namespace std::placeholders;
        auto sp = this->shared_from_this();
        if (!sp) return;
        m_socket.get_io_service().post([sp](){
                sp->m_stop = false;
                sp->m_msgio.request("get_image", std::make_tuple(), [sp](auto&&...args) {sp->on_image_arrived(std::forward<decltype(args)>(args)...);});
            });
    }
    void stop_video() {
        auto sp = this->shared_from_this();
        if (!sp) return;
        m_socket.get_io_service().post([sp](){
                sp->m_stop = true;
            });
    }
    void on_image_arrived(boost::system::error_code const & ec, msgpack::object_handle oh) {
        if (m_stop) return;
        if (oh.get().type == msgpack::type::NIL) return;
        if (ec) return;
        auto sp = this->shared_from_this();
        if (!sp) return;
        std::stringstream ss;
        ss << "client-" << sp ;
        try {
            image_sink().show(ss.str(), oh.get().as<cv::Mat>());
        } catch(...) {
            return;
        }
        m_msgio.request("get_image",std::make_tuple(), [sp](auto&&...args) {sp->on_image_arrived(std::forward<decltype(args)>(args)...);});
    }
private:
    void do_exit() {
        m_socket.get_io_service().post(
            [this]() mutable
            {
                auto sp = this->shared_from_this();
                sp->m_sessions.erase(sp);
                std::cerr << "[info] client #" << sp << " exit." << std::endl;
            });
        m_socket.close();
    }
    bool m_stop;
    boost::asio::ip::tcp::socket m_socket;
    session_manager & m_sessions;
    api::message_io_object m_msgio;
    api::Detect & m_d;
};

#endif // API_SRC_SIMPLE_SERVER_SESSION_HPP_INCLUDED
