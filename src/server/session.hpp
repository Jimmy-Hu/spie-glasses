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
#include <api/Detect.hpp>
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
        , m_mgr(mgr)
        , m_msgio(m_socket)
        , m_d(d)
        {}

    void start() {
        auto sp = this->shared_from_this();
        std::cerr << "[info] client #" << sp << " start." << std::endl;
        m_mgr.start(sp);
        m_msgio.set_error_handler([this](auto const & ){this->do_exit();});
        m_msgio.set_request_handler<int(int,int)>("add",std::plus<int>{});
        m_msgio.set_request_handler<int(int,int)>("sub",std::minus<int>{});
        m_msgio.set_request_handler<int(int,int)>("div",std::divides<int>{});
        m_msgio.set_request_handler<int(int,int)>("mul",std::multiplies<int>{});
        m_msgio.set_post_handler<void(std::string,std::size_t)>
            ("login",[this](auto&&...args){this->on_login(std::forward<decltype(args)>(args)...);});
        m_msgio.set_post_handler<void(std::vector<std::size_t>)>
            ("hint",[this](auto&&...args){this->on_hint(std::forward<decltype(args)>(args)...);});
        m_msgio.set_post_handler<void(cv::Mat)>
            ("put_image",[this](auto&&...args){this->on_image_arrived(std::forward<decltype(args)>(args)...);});
        m_msgio.set_post_handler<void(std::vector<std::size_t>)>
            ("hint_ids",[this](auto&&...args){this->on_hint_ids(std::forward<decltype(args)>(args)...);});
        m_msgio.set_post_handler<void(std::vector<std::string>)>
            ("hint_names",[this](auto&&...args){this->on_hint_names(std::forward<decltype(args)>(args)...);});
        m_msgio.async_dispatch(this->shared_from_this());
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
        auto rsp = m_mgr.get_room(sp);
        if (!rsp) return;
        auto isp = rsp->image_client.lock();
        if (!isp) return;
        m_socket.get_io_service().post([sp,rsp,isp](){
                sp->m_stop = false;
                isp->m_msgio.post("image_start", std::make_tuple());
            });
    }
    void stop_video() {
        using namespace std::placeholders;
        auto sp = this->shared_from_this();
        if (!sp) return;
        m_socket.get_io_service().post([sp](){
                sp->m_stop = true;
                sp->m_msgio.post("image_stop", std::make_tuple());
            });
    }
    void on_image_arrived(cv::Mat image) {
        if (m_stop) return;
        auto sp = shared_from_this();
        do {
            if (!sp) break;
            auto rsp = m_mgr.get_room(sp);
            if (!rsp) break;
            auto asp = rsp->audio_client.lock();
            if (!asp) break;
            asp->m_msgio.post("people_names", std::make_tuple(m_d.detect_user(image)));
        } while (false);
        std::stringstream ss;
        ss << "client-" << sp;
        image_sink().show(ss.str(), image);
    }
    std::vector<std::size_t> generate_ramdom_ids() {
        std::vector<std::size_t> v(std::rand() % 5);
        std::generate(v.begin(), v.end(), std::rand); 
        return v;
    }
    void on_login(std::string s, std::size_t id) {
        auto sp = shared_from_this();
        m_mgr.login(sp, std::move(s), id);
        show_video();
    }
    void on_hint(std::vector<std::size_t> ids) {
        auto sp = shared_from_this();
        if (!sp) return;
        auto rsp = m_mgr.get_room(sp);
        if (!rsp) return;
        auto asp = rsp->audio_client.lock();
        if (!asp) return;
        asp->m_msgio.post("people_ids", ids);
    }
    void on_hint_ids(std::vector<std::size_t> ids) {
        auto sp = shared_from_this();
        if (!sp) return;
        auto rsp = m_mgr.get_room(sp);
        if (!rsp) return;
        auto asp = rsp->audio_client.lock();
        if (!asp) return;
        asp->m_msgio.post("people_names", std::make_tuple(ids));
    }
    void on_hint_names(std::vector<std::string> names) {
        auto sp = shared_from_this();
        if (!sp) return;
        auto rsp = m_mgr.get_room(sp);
        if (!rsp) return;
        auto asp = rsp->audio_client.lock();
        if (!asp) return;
        asp->m_msgio.post("people_names", std::make_tuple(names));
    }
private:
    void do_exit() {
        m_socket.get_io_service().post(
            [sp = this->shared_from_this()]() mutable
            {
                sp->m_mgr.remove(sp);
                std::cerr << "[info] client #" << sp << " exit." << std::endl;
            });
        m_socket.close();
    }
    bool m_stop;
    boost::asio::ip::tcp::socket m_socket;
    session_manager & m_mgr;
    api::message_io_object m_msgio;
    api::Detect & m_d;
};

#endif // API_SRC_SIMPLE_SERVER_SESSION_HPP_INCLUDED
