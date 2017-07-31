#ifndef API_MESSAGE_IO_OBJECT_HPP_INCLUDED
#define API_MESSAGE_IO_OBJECT_HPP_INCLUDED


// internal api
#include <api/asio_msgpack.hpp>
#include <api/msgpack_reader.hpp>
#include <api/msgpack_writer.hpp>
#include <api/requester.hpp>
#include <api/responser.hpp>
#include <api/dispatcher.hpp>
// external api
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <msgpack.hpp>
// stdandard api
#include <iostream>
#include <map>
#include <utility>
#include <memory>

namespace api {
    struct message_io_object
    {
    public:
        using command_type = msgpack::object_handle;
        message_io_object(boost::asio::ip::tcp::socket & socket)
            : m_qw(socket)
            , m_r(socket)
            , m_req(m_qw)
            , m_res(m_qw)
            , m_dis(m_r,m_req,m_res)
            {}
        boost::asio::io_service & get_io_service() {
            return m_r.get_io_service();
        }
        boost::asio::io_service const & get_io_service() const {
            return m_r.get_io_service();
        }
        template<typename TagT, typename TupleT, typename HandlerT>
        void request(TagT && tag, TupleT const & t, HandlerT h) {
            m_req.request(std::forward<TagT>(tag), t, std::move(h));
        }
        template<typename TagT, typename TupleT, typename HandlerT>
        void post(TagT && tag, TupleT const & t, HandlerT h) {
            m_req.post(std::forward<TagT>(tag), t, std::move(h));
        }
        template<typename TagT, typename TupleT>
        void post(TagT && tag, TupleT const & t) {
            m_req.post(std::forward<TagT>(tag), t);
        }
        template<typename HandlerT>
        void request_raw(std::string const & s, std::vector<std::uint8_t> const & t, HandlerT h) {
            m_req.request_raw(s, t, std::move(h));
        }
        void post_raw(std::string const & s, std::vector<std::uint8_t> const & t) {
            m_req.post_raw(s, t);
        }
        template <typename SignatureT>
        void set_request_handler(std::string const & s, std::function<SignatureT> f) {
            m_res.set_request_handler(s, std::move(f));
        }
        template <typename SignatureT>
        void set_post_handler(std::string const & s, std::function<SignatureT> f) {
            m_res.set_post_handler(s, std::move(f));
        }
        template <typename ...ArgTs>
        void async_dispatch(ArgTs&&...args) {
            m_dis.async_dispatch(std::forward<ArgTs>(args)...);
        }
        void set_error_handler(std::function<void(boost::system::error_code const &)> fn) {
            m_dis.set_error_handler(std::move(fn));
        }
        // template <typename SignatureT>
        // void set_request_handler(std::size_t n, std::function<SignatureT> f) {
        //     m_res.set_request_handler(n, std::move(f));
        // }
    private:
        api::msgpack_queued_writer m_qw;
        api::msgpack_reader m_r;
        api::requester m_req;
        api::responser m_res;
        api::dispatcher m_dis;
    };
} // namespace api
#endif // API_MESSAGE_IO_OBJECT_HPP_INCLUDED
