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
           const boost::asio::ip::tcp::endpoint& endpoint,
           api::Detect & d
        )
        : m_acceptor(io_service, endpoint)
        , m_socket(io_service)
        , m_d(d)
        {
            do_accept();
        }
    void get_focus_session() {
        m_acceptor.get_io_service().post(
            [this](){
                auto sp = this->m_wp.lock();
                std::cerr << "[info] #" << sp << std::endl;
            });
    }
    void set_focus_session(std::uintptr_t p) {
        m_acceptor.get_io_service().post(
            [this,p](){
                if (p == 0) {
                    session_ptr sp;
                    if (m_mgr.begin() != m_mgr.end()) {
                        sp = m_mgr.begin()->first;
                        set_focus_session(reinterpret_cast<std::uintptr_t>(sp.get()));
                    }
                    std::cerr << "[info] session set to #" << sp << std::endl;
                    m_wp = sp;
                    return;
                }
                auto i = m_mgr.find(p);
                if (i == m_mgr.end()) {
                    std::cerr << "[info] session not found." << std::endl;
                    return;
                }
                session_ptr sp = i->first;
                if (reinterpret_cast<std::uintptr_t>(sp.get()) == p) {
                    std::cerr << "[info] session set to #" << sp << std::endl;
                    m_wp = sp;
                }
                else {
                    std::cerr << "[info] session not found." << std::endl;
                    return;
                }
            });
    }
    void list_sessions() {
        m_acceptor.get_io_service().post(
            [this](){
                for (auto & kv : m_mgr) {
                    std::cerr << "[info] #" << kv.first << "\n";
                }
                std::cerr << std::flush;
            });
    }
    void write(std::vector<std::string> const & v) {
        auto n = v.size();
        if (n == 0) return;
        if (v[0].size() == 0) return;
        if (v[0][0] == ':') {
            if (v[0] == ":ls") {
                list_sessions();
            }
            else if (v[0] == ":set") {
                if (n == 3 && v[1] == "cid") {
                    try {
                        std::uintptr_t cid;
                        if (v[1].size() > 0 && v[1][0] == '#') {
                            cid = std::stoull(v[1].substr(1));
                        } else if (v[2] == "default") {
                            cid = 0;
                        } else {
                            cid = std::stoull(v[2]);
                        }
                        set_focus_session(cid);
                    }
                    catch(...) {}
                }
            }
            else if (v[0] == ":get") {
                if (n == 2 && v[1] == "cid") {
                    get_focus_session();
                }
            }
            else if (v[0] == ":show") {
                // if (n == 2 && v[1] == "video") {
                    auto sp = m_wp.lock();
                    if (!sp) return;
                    std::cerr << "[info] show" << std::endl;
                    sp->show_video();
                // }
            }
            else if (v[0] == ":stop") {
                // if (n == 2 && v[1] == "video") {
                    auto sp = m_wp.lock();
                    if (!sp) return;
                    std::cerr << "[info] stop" << std::endl;
                    sp->stop_video();
                // }
            }
        }
        else {
            auto sp = m_wp.lock();
            if (!sp) return;
            sp->write(v);
        }
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
                    auto sp = std::make_shared<session>(std::move(m_socket),m_mgr,m_d);
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
    api::Detect & m_d;
};

#endif // API_SRC_SIMPLE_SERVER_SERVER_HPP_INCLUDED
