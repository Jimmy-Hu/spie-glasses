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
class chat_session;
using session_manager = std::set<std::shared_ptr<chat_session>>;
class chat_session
    : public std::enable_shared_from_this<chat_session>
{
public:
    chat_session(tcp::socket socket, session_manager & mgr)
        : m_socket(std::move(socket))
        , m_sessions(mgr)
        , m_unp()
        , m_rcmd() {}
    void start() {
        auto sp = this->shared_from_this();
        std::cerr << "[info] client #" << sp << " start." << std::endl;
        m_sessions.insert(sp);
        do_read();
    }
private:
    void do_read() {
        api::async_read_msgpack(
            m_socket,
            m_unp,
            m_rcmd,
            [this](boost::system::error_code const & ec, std::size_t /*length*/) mutable {
                if (!ec) {
                    auto sp = this->shared_from_this();
                    std::cerr << "[info][read][#" << sp << "] object=" << m_rcmd.get() << std::endl;
                    // m_rcmd.clear();
                    do_read();
                }
                else {
                    do_exit();
                }
            });
    }
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
    using command_type = msgpack::object_handle;
    msgpack::unpacker m_unp;
    tcp::socket m_socket;
    command_type m_rcmd; // read_command
    session_manager & m_sessions;
};

//----------------------------------------------------------------------

class chat_server
{
public:
    chat_server(boost::asio::io_service& io_service,
                const tcp::endpoint& endpoint)
        : m_acceptor(io_service, endpoint),
          m_socket(io_service)
        {
            do_accept();
        }

private:
    void do_accept() {
        m_acceptor.async_accept(
            m_socket,
            [this](boost::system::error_code ec) {
                if (!ec) {
                    auto sp = std::make_shared<chat_session>(std::move(m_socket),m_sessions);
                    sp->start();
                    std::cout << "[info] accept client-" << sp << std::endl;
                }
                do_accept();
            });
    }
    tcp::acceptor m_acceptor;
    tcp::socket m_socket;
    session_manager m_sessions;
};

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: chat_server <port> [<port> ...]\n";
            return 1;
        }

        boost::asio::io_service io_service;

        std::list<chat_server> servers;
        for (int i = 1; i < argc; ++i)
        {
            tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
            servers.emplace_back(io_service, endpoint);
        }

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
