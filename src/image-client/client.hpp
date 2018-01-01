#ifndef API_SRC_IMAGE_CLIENT_CLIENT_HPP_INCLUDED
#define API_SRC_IMAGE_CLIENT_CLIENT_HPP_INCLUDED

// internal api
#include <api/asio_msgpack.hpp>
#include <api/message_io_object.hpp>
#include <api/msgpack_reader.hpp>
#include <api/msgpack_writer.hpp>
#include <api/cv_msgpack.hpp>
#include <api/compressed_mat.hpp>
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

using boost::asio::ip::tcp;
using json = nlohmann::json ;

class client
{
public:
    using command_type = std::vector<std::string>;
    /*    client constructor 
     */
    client(boost::asio::io_service& io_service,
           tcp::resolver::iterator endpoint_iterator,
           cv::VideoCapture & cap
        )
        : m_ios(io_service)
        , m_socket(io_service)
        , m_msgio(m_socket)
        , m_cap(cap)
        , m_start(false)
        , m_mode(2)
    {
        do_connect(endpoint_iterator);
        m_msgio.set_error_handler([this](auto const & ec){this->do_exit();});
        m_msgio.set_request_handler<cv::Mat()>("get_image", [this](){return this->get_image();});
        m_msgio.set_request_handler<void()>("image_start", [this](){return this->on_image_start();});
        m_msgio.set_request_handler<void()>("image_stop", [this](){return this->on_image_stop();});
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
    void set_mode(int mode) {
        m_ios.post([this,mode]() mutable {
                m_mode = mode;
            });
    }
private:
    /*    get_image function
     */
    cv::Mat get_image() {
        cv::Mat frame;
        m_cap.read(frame);
        return frame;
    }
    void on_image_start() {
        if (m_start) return;
        m_start = true;
        do_put_image();
    }
    void on_image_stop() {
        m_start = false;
    }
    void do_put_image() {
        api::compressed_mat frame;
        if (this->m_start && m_cap.isOpened())
            m_cap.read(frame);
        frame.mode = m_mode;
        m_msgio.post("put_image", std::make_tuple(frame),
                     [this](boost::system::error_code const & ec) {
                         if (ec) this->close();
                         else this->do_put_image();
                     });
    }
    void login() {
        m_msgio.post("login", std::make_tuple("image-client", 1));
    }
    void on_connected() {
        m_msgio.set_post_handler<void()>("image_start", [this](){this->on_image_start();});
        m_msgio.set_post_handler<void()>("image_stop", [this](){this->on_image_stop();});
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
    cv::VideoCapture & m_cap;
    bool m_start;
    int m_mode;
};

#endif // API_SRC_IMAGE_CLIENT_CLIENT_HPP_INCLUDED
