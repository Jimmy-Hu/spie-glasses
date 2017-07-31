#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <api/asio_msgpack.hpp>
#include <api/parser.hpp>
#include <json.hpp>

using boost::asio::ip::tcp;
using json = nlohmann::json ;

// typedef std::deque<chat_message> chat_message_queue;

class chat_client
{
public:
    using command_type = std::vector<std::string>;
    chat_client(boost::asio::io_service& io_service,
                tcp::resolver::iterator endpoint_iterator)
        : m_ios(io_service),
          m_work(io_service),
          m_socket(io_service),
          m_qw(m_socket),
          m_qr(m_socket),
          m_req(m_qw),
          m_res(m_qw),
          m_dis(m_qr,m_req,m_res)
    {
        do_connect(endpoint_iterator);
        m_dis.set_error_handler([this](auto const & ec){this->do_exit();});
    }

    // void write(command_type v) {
    //     // std::cerr << "[debug] client.write " << std::endl;
    //     auto pbuf = std::make_shared<msgpack::sbuffer>();
    //     if (!pbuf) return;
    //     msgpack::pack(*pbuf, v);
    //     m_qw.async_write(*pbuf, [pbuf](boost::system::error_code const &){});
    // }


    void close() {
        m_ios.post([this]() { m_socket.close(); });
    }
    void write(std::vector<std::string> const & v) {
        if (v.size() < 2) return;
        try {
            auto raw = json::to_msgpack(json::parse(v[1]));
            m_req.request_raw(v[0], raw, [](boost::system::error_code const & ec, msgpack::object_handle oh) {
                    if (!ec) std::cerr << "[info][response] object=" << oh.get() << std::endl;
                    else std::cerr << "[info][response] failed" << std::endl;
                });
            std::cerr << "[info][request] start" << std::endl;
        } catch(...) {
            std::cerr << "[info][request] wrong argument (json parsing error)" << std::endl;
        }
    }

private:
    // void do_read() {
        // m_qr.async_read(
        //     m_oh,
        //     [this](boost::system::error_code const & ec) mutable {
        //         if (!ec) {
        //             std::cerr << "[info][read] object=" << m_oh.get() << std::endl;
        //             do_read();
        //         }
        //         else
        //             m_socket.close();
        //     });
    // }
    void do_connect(tcp::resolver::iterator endpoint_iterator)
        {
            boost::asio::async_connect(
                m_socket, endpoint_iterator,
                [this](boost::system::error_code const & ec, tcp::resolver::iterator)
                {
                    auto h = [](boost::system::error_code const & ec, msgpack::object_handle oh){
                        if (!ec) std::cerr << "[info][response] object=" << oh.get() << std::endl;
                    };
                    if (!ec) {
                        m_req.request("add", std::make_tuple(1,2),h);
                        m_req.request("mul", std::make_tuple(1,4),h);
                        m_req.request("sub", std::make_tuple(1,2),h);
                        m_req.request("div", std::make_tuple(1,4),h);
                        m_req.request("mod", std::make_tuple(1,4),h);
                        m_dis.async_dispatch();
                    }
                    else m_socket.close();
                });
        }
    void do_exit() {
        m_socket.close();
    }
private:
    boost::asio::io_service& m_ios;
    boost::asio::io_service::work m_work;
    tcp::socket m_socket;
    api::tcp_socket_queued_sbuffer_writer m_qw;
    api::tcp_socket_queued_msgpack_handle_reader m_qr;
    api::requester m_req;
    api::responser m_res;
    api::dispatcher m_dis;
    msgpack::object_handle m_oh;
};

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: chat_client <host> <port>\n";
            return 1;
        }

        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        auto endpoint_iterator = resolver.resolve({ argv[1], argv[2] });
        chat_client c(io_service, endpoint_iterator);

        std::thread t(
            [&io_service](){
                io_service.run();
                std::cerr << "[thread] ending" << std::endl;
            });
        std::string s;
        bool continued = false;
        api::command_parser cp;
        while (true) {
            if (!std::getline(std::cin,s))
                break;
            cp.parse(s.begin(),s.end());
            if (cp.is_complete()) {
                auto v = cp.get();
                c.write(std::move(v));
                cp.clear();
                continued = false;
            }
            else {
                continued = true;
            }
        }
        c.close();
        t.join();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
