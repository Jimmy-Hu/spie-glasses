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
// external api
#include <msgpack.hpp>
#include <boost/asio.hpp>
// standard api
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <map>
#include <utility>
#include <algorithm>

class session
    : public std::enable_shared_from_this<session>
{
public:
    using command_type = msgpack::object_handle;
    session(boost::asio::ip::tcp::socket socket, session_manager & mgr)
        : m_socket(std::move(socket))
        , m_mgr(mgr)
        , m_r(m_socket)
        , m_msgio(m_socket)
        {}

    void start() {
        auto sp = this->shared_from_this();
        std::cerr << "[info] client #" << sp << " start." << std::endl;
        m_mgr.start(sp);
        // auto spoh = std::make_shared<msgpack::object_handle>();
        m_msgio.async_dispatch(this->shared_from_this());
        // do_read();
        // do_read();
        // m_msgio.set_error_handler([this](auto const & ec){this->do_exit();});
        // m_msgio.async_dispatch();
    }
    void do_read() {
        auto sp = this->shared_from_this();
        auto spoh = std::make_shared<msgpack::object_handle>();
        m_r.async_read(*spoh,[sp,spoh](auto && ec) mutable {
                if (ec) std::cerr << "error " << ec.value()  << std::endl;
                else {
                    std::cerr << spoh->get() << std::endl;
                    sp->do_read();
                }
        });
    }
    void close() {
        m_socket.get_io_service().post([this](){ m_socket.close();});
    }
private:
    void do_exit() {
        m_socket.get_io_service().post(
            [this]() mutable
            {
                auto sp = this->shared_from_this();
                sp->m_mgr.remove(sp);
                std::cerr << "[info] client #" << sp << " exit." << std::endl;
            });
        m_socket.close();
    }
    boost::asio::ip::tcp::socket m_socket;
    session_manager & m_mgr;
    api::msgpack_reader m_r;
    api::message_io_object m_msgio;
};

#endif // API_SRC_SIMPLE_SERVER_SESSION_HPP_INCLUDED
