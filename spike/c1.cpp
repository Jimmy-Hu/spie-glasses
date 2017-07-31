#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <api/asio_msgpack.hpp>
#include <api/parser.hpp>

using boost::asio::ip::tcp;

// typedef std::deque<chat_message> chat_message_queue;

class chat_client
{
public:
    using command_type = std::vector<std::string>;
    chat_client(boost::asio::io_service& io_service,
                tcp::resolver::iterator endpoint_iterator)
        : m_ios(io_service),
          m_socket(io_service),
          m_work(io_service)
    {
        do_connect(endpoint_iterator);
    }

    void write(command_type v) {
        std::cerr << "[debug] write " << std::endl;
        m_ios.post(
            [this, cmd = std::move(v)]()
            {
                bool write_in_progress = !m_wcmds.empty();
                m_wcmds.push_back(cmd);
                if (!write_in_progress)
                {
                    do_write();
                }
            });
    }


    void close() {
        m_ios.post([this]() { m_socket.close(); });
    }

private:
    void do_connect(tcp::resolver::iterator endpoint_iterator)
        {
            boost::asio::async_connect(
                m_socket, endpoint_iterator,
                [this](boost::system::error_code const &ec, tcp::resolver::iterator)
                {
                    // if (!ec)
                    // {
                    //     do_read();
                    // }
                });
        }

    // void do_read() {
    //     api::async_read_msgpack(
    //         m_socket,
    //         m_unp,
    //         m_rcmd,
    //         [this](boost::system::error_code const &ec, std::size_t /*length*/) mutable
    //         {
    //             if (!ec) {
    //                 std::cout << m_rcmd << std::endl;
    //                 do_read();
    //             }
    //             else {
    //                 m_socket.close();
    //             }
    //         });
    // }

    void do_write() {
        std::cerr << "[debug] do_write " << std::endl;
        api::async_write_msgpack(
            m_socket,
            m_wcmds.front(),
            [this](boost::system::error_code const & ec, std::size_t /*length*/) mutable
            {
                std::cerr << "[debug] async_write_msgpack done " << std::endl;
                if (!ec) {
                    m_wcmds.pop_front();
                    if (!m_wcmds.empty()) {
                        do_write();
                    }
                }
                else {
                    m_socket.close();
                }
            });
    }

private:
    // using command_type = std::vector<std::string>;
    // msgpack::unpacker m_unp;
    boost::asio::io_service& m_ios;
    boost::asio::io_service::work m_work;
    tcp::socket m_socket;
    // command_type m_rcmd; // read_command
    std::deque<command_type> m_wcmds; // write_commands

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
                msgpack::sbuffer sbuf;
                auto v = cp.get();
                msgpack::pack(sbuf, v);
                std::cout.write(sbuf.data(),sbuf.size());
                std::cout.flush();
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
// #include <functional>
// template <typename T>
// struct pipe
// {
//     using handler_type = std::function<void(boost::system::error_code const &)>;
//     using pending_read_pair = std::pair<std::reference_wrapper<T>, handler_type>;
//     using pending_write_pair = std::pair<std::reference_wrapper<T const>, handler_type>;
//     using pending_write_queue = std::deque<pending_write_pair>;
//     using pending_read_queue = std::deque<pending_read_pair>;
//     boost::asio::io_service & m_ios;
//     pending_read_queue m_rq; // read_queue
//     pending_write_queue m_wq; // write_queue
//     pipe(boost::asio::io_service & ios) : m_ios(ios) {}
//     void async_read(T & t, handler_type h) {
//         m_ios.post(
//             [&t, hh = std::move(h)](){
//                 do_async_read(t,std::move(hh));
//             });
//     }
//     void do_async_read(T & rt, handler_type rh) {
//         m_rq.push_back({rt, std::move(rh)});
//         if (m_wq.empty()) return;
//         do_comsume();
//     }
//     void async_write(T const & t, handler_type h) {
//         m_ios.post(
//             [&t, hh = std::move(h)](){
//                 do_async_write(t,std::move(hh));
//             });
//     }
//     void do_async_write(T const & wt, handler_type wh) {
//         m_wq.push_back({wt, std::move(wh)});
//         if (m_rq.empty()) return;
//         do_consume();
//     }
//     void do_consume() {
//         m_rq.front().first.get() = m_wq.front().first.get();
//         m_rq.front().second(boost::system::error_code());
//         m_wq.front().second(boost::system::error_code());
//     }
//     void do_cancel() {
//         for (auto & rp : m_rq) {
//             rp.second(boost::system::error_code(boost::system::errc::operation_canceled));
//         }
//         for (auto & rp : m_rq) {
//             rp.second(boost::system::error_code(boost::system::errc::operation_canceled));
//         }
//     }
// };
