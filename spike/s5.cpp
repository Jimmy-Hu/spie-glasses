#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>
#include <api/asio_msgpack.hpp>
#include <api/cv_msgpack.hpp>
#include <api/parser.hpp>
#include <thread>
#include <algorithm>
#include <json.hpp>


using json = nlohmann::json ;

struct image_shower
{

    boost::asio::io_service m_ios;
    std::thread m_thread;
    bool m_is_running; // 
    image_shower()
        : m_ios()
        , m_thread()
        , m_is_running(false)
        {}
    void open() {
        m_is_running = true;
        m_thread = std::thread([this](){do_run();});
    }
    void show(std::string const & s, cv::Mat m)
        {
            m_ios.post(
                [this,s,m]() {
                    cv::imshow(s,m);
                });
        }
    void close() {
        m_ios.post([this](){m_is_running = false;});
        m_thread.join();
    }
    void do_run() {
        while (m_is_running) {
            m_ios.run();
            cv::waitKey(27);
        }
    }
    bool is_opened() {
        return m_is_running;
    }
};
image_shower & shower() {
    static image_shower s;
    return s;
}

template<class T>
struct pointer_comp {
    typedef std::true_type is_transparent;
    // helper does some magic in order to reduce the number of
    // pairs of types we need to know how to compare: it turns
    // everything into a pointer, and then uses `std::less<T*>`
    // to do the comparison:
    struct helper {
        T * ptr;
        helper():ptr(nullptr) {}
        helper(helper const&) = default;
        helper(T* p):ptr(p) {}
        template<typename ...Ts>
        helper( std::shared_ptr<Ts...> const& sp ) : ptr(sp.get()) {}
        template<typename...Ts>
        helper( std::unique_ptr<Ts...> const& up ) : ptr(up.get()) {}
        // && optional: enforces rvalue use only
        bool operator<( helper h ) const {
            return false;
            return std::less<T*>()( ptr, h.ptr );
        }
    };
    // without helper, we'd need 2^n different overloads, where
    // n is the number of types we want to support (so, 8 with
    // raw pointers, unique pointers, and shared pointers).  That
    // seems silly:
    // && helps enforce rvalue use only
    bool operator()( helper const&& lhs, helper const&& rhs ) const {
      return lhs < rhs;
    }
};

template<class T>
struct uintptr_comp {
    typedef std::true_type is_transparent;
    // helper does some magic in order to reduce the number of
    // pairs of types we need to know how to compare: it turns
    // everything into a pointer, and then uses `std::less<T*>`
    // to do the comparison:
    struct helper {
        T * ptr;
        helper():ptr(nullptr) {}
        helper(helper const&) = default;
        helper(T* p):ptr(p) {}
        helper(std::uintptr_t p):ptr(reinterpret_cast<T*>(p)) {}
        template<typename ...Ts>
        helper( std::shared_ptr<Ts...> const& sp ) : ptr(sp.get()) {}
        template<typename...Ts>
        helper( std::unique_ptr<Ts...> const& up ) : ptr(up.get()) {}
        // && optional: enforces rvalue use only
        bool operator<( helper h ) const {
            return false;
            return std::less<T*>()( ptr, h.ptr );
        }
    };
    // without helper, we'd need 2^n different overloads, where
    // n is the number of types we want to support (so, 8 with
    // raw pointers, unique pointers, and shared pointers).  That
    // seems silly:
    // && helps enforce rvalue use only
    bool operator()( helper const&& lhs, helper const&& rhs ) const {
      return lhs < rhs;
    }
};

//----------------------------------------------------------------------
class chat_session;
using session_ptr = std::shared_ptr<chat_session>;
// using session_manager = std::set<session_ptr>;
using session_manager = std::set<session_ptr,uintptr_comp<chat_session>>;
class chat_session
    : public std::enable_shared_from_this<chat_session>
{
public:
    using command_type = msgpack::object_handle;
    chat_session(boost::asio::ip::tcp::socket socket, session_manager & mgr)
        : m_stop(true)
        , m_socket(std::move(socket))
        , m_sessions(mgr)
        , m_qw(m_socket)
        , m_qr(m_socket)
        , m_req(m_qw)
        , m_res(m_qw)
        , m_dis(m_qr,m_req,m_res)
        , m_oh()
        {}

    void start() {
        auto sp = this->shared_from_this();
        std::cerr << "[info] client #" << sp << " start." << std::endl;
        m_sessions.insert(sp);
        m_dis.set_error_handler([this](auto const & ec){this->do_exit();});
        m_res.set_request_handler<int(int,int)>("add",std::plus<int>{});
        m_res.set_request_handler<int(int,int)>("sub",std::minus<int>{});
        m_res.set_request_handler<int(int,int)>("div",std::divides<int>{});
        m_res.set_request_handler<int(int,int)>("mul",std::multiplies<int>{});
        // m_res.set_request_handler<int(std::vector<int> &)>("add",[this](std::vector<int> & v){return std::accumulate(v.begin(),v.end());});
        m_dis.async_dispatch();
        // do_read();
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
            m_req.request_raw(v[0], raw, [sp](boost::system::error_code const & ec, msgpack::object_handle oh) {
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
                sp->m_req.request("get_image", std::make_tuple(), [sp](auto&&...args) {sp->on_image_arrived(std::forward<decltype(args)>(args)...);});
            });
    }
    void stop_video() {
        using namespace std::placeholders;
        auto sp = this->shared_from_this();
        if (!sp) return;
        m_socket.get_io_service().post([sp](){
                sp->m_stop = true;
            });
    }
    void on_image_arrived(boost::system::error_code const & ec, msgpack::object_handle oh) {
        using namespace std::placeholders;
        if (m_stop) return;
        if (oh.get().type == msgpack::type::NIL) return;
        if (ec) return;
        auto sp = this->shared_from_this();
        if (!sp) return;
        std::stringstream ss;
        ss << "client-" << sp ;
        try {
            // cv::imshow(ss.str(), oh.get().as<cv::Mat>());
            shower().show(ss.str(), oh.get().as<cv::Mat>());
        } catch(...) {
            return;
        }
        m_req.request("get_image",std::make_tuple(), [sp](auto&&...args) {sp->on_image_arrived(std::forward<decltype(args)>(args)...);});
    }
private:
    // void do_read() {
        // m_qr.async_read(
        //     m_oh,
        //     [this](boost::system::error_code const & ec) mutable {
        //         if (!ec) {
        //             auto sp = this->shared_from_this();
        //             std::cerr << "[info][read][#" << sp << "] object=" << m_oh.get() << std::endl;
        //             do_read();
        //         }
        //         else do_exit();
        //     });
        // m_dis.async_dispatch();
    // }
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
    // using command_type = std::vector<std::string>;
    bool m_stop;
    boost::asio::ip::tcp::socket m_socket;
    session_manager & m_sessions;
    api::tcp_socket_queued_sbuffer_writer m_qw;
    api::tcp_socket_queued_msgpack_handle_reader m_qr;
    api::requester m_req;
    api::responser m_res;
    api::dispatcher m_dis;
    command_type m_oh;
};

// //----------------------------------------------------------------------

class chat_server
{
public:
    chat_server(boost::asio::io_service& io_service,
                const boost::asio::ip::tcp::endpoint& endpoint)
        : m_acceptor(io_service, endpoint),
          m_socket(io_service)
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
                    if (m_sessions.begin() != m_sessions.end()) {
                        sp = *m_sessions.begin();
                        set_focus_session(reinterpret_cast<std::uintptr_t>(sp.get()));
                    }
                    std::cerr << "[info] session set to #" << sp << std::endl;
                    m_wp = sp;
                    return;
                }
                auto i = m_sessions.find(p);
                if (i == m_sessions.end()) {
                    std::cerr << "[info] session not found." << std::endl;
                    return;
                }
                auto sp = *i;
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
                for (auto & session : m_sessions) {
                    std::cerr << "[info] #" << session << "\n";
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
                for (auto session : m_sessions) {
                    session->close();
                }
                m_sessions.clear();
                m_acceptor.close();
            });
    }
private:
    void do_accept() {
        m_acceptor.async_accept(
            m_socket,
            [this](boost::system::error_code ec) {
                if (!ec) {
                    auto sp = std::make_shared<chat_session>(std::move(m_socket),m_sessions);
                    sp->start();
                    std::cerr << "[info] accept client-" << sp << std::endl;
                    do_accept();
                } else {
                    std::cerr << "[info] accept stop" << std::endl;
                }
            });
    }
    std::weak_ptr<chat_session> m_wp; // current session
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket m_socket;
    session_manager m_sessions;
};

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: chat_server <port>\n";
            return 1;
        }

        boost::asio::io_service io_service;

        // std::list<chat_server> servers;
        // for (int i = 1; i < argc; ++i)
        // {
        // servers.emplace_back(io_service, endpoint);
        // }
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), std::atoi(argv[1]));
        chat_server s(io_service,endpoint);
        std::thread t([&]() { io_service.run();});
        shower().open();
        std::string str;
        api::command_parser cp;
        bool continued = false;
        while (true) {
            if (!std::getline(std::cin,str))
                break;
            if (str == "exit") {
                std::cerr << "[info] exit server" << std::endl;
                s.close();
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
        t.join();
        shower().close();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
