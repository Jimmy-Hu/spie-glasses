//
// Copyright (C) 2017 CheinYu Lin. All rights reserved.
//
#ifndef API_DISPATCHER_HPP_INCLUDED
#define API_DISPATCHER_HPP_INCLUDED

// internal api
#include <api/requester.hpp>
#include <api/responser.hpp>
#include <api/msgpack_reader.hpp>
// external api
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <msgpack.hpp>
// standard api
#include <memory>
#include <utility>
#include <functional>

namespace api {
    class dispatcher {
    public:
        dispatcher(msgpack_reader & reader, requester & req, responser & res)
            : m_reader(reader)
            , m_req(req)
            , m_res(res)
            {}
        template <typename... ArgTs>
        void async_dispatch(ArgTs&&...args) {
            auto sp = std::make_shared<msgpack::object_handle>();
            m_reader.async_read(
                *sp,
                [=](boost::system::error_code const & ec) mutable {
                    do_dispatch(ec,std::move(*sp),std::forward<ArgTs>(args)...);
                });
        }
        void set_error_handler(std::function<void(boost::system::error_code const &)> fn) {
            m_eh = std::move(fn);
        }
    private:
        void do_close(boost::system::error_code const & ec) {
            m_req.close(ec);
            m_eh(ec);
        }
        bool assert_(bool cond, char const * msg) {
            if (cond) return cond;
            std::cerr << "[info][dispatcher] assert failure: " << msg << ", closing..."  << std::endl;
            do_close(m_protocol_error);
            return cond;
        }
        template <typename ...ArgTs>
        void do_dispatch(boost::system::error_code const & ec, msgpack::object_handle oh, ArgTs && ...args) {
            if (ec) {
                std::cerr << "[info][dispatcher] io error, value= " << ec.value() << std::endl;
                m_req.close(ec);
                if (m_eh) m_eh(ec);
                return;
            }
            async_dispatch(std::forward<ArgTs>(args)...);
            auto o = oh.get();
            enum {REQUEST = 1, RESPONSE = 2, POST = 3};
            std::uint64_t type;
            // std::uint64_t id;
            // std::cerr << "[info] dispatch object=" << o << std::endl;
            if (!assert_(o.type == msgpack::type::ARRAY, "invalid message, message is not a array object")) return;
            if (!assert_(o.via.array.size == 3, "invalid message, array size should be 3")) return;
            if (!assert_(o.via.array.ptr[0].type == msgpack::type::POSITIVE_INTEGER, "invalid message, message.type is not a positive object")) return;
            o.via.array.ptr[0].convert(type);
            switch (type) {
            case REQUEST:
                m_res.handle_request(ec,std::move(oh));
                break;
            case RESPONSE:
                m_req.handle_response(ec,std::move(oh));
                break;
            case POST:
                m_res.handle_post(ec,std::move(oh));
                break;
            default: ;
            }
        }
        std::function<void(boost::system::error_code const &)> m_eh;
        msgpack_reader & m_reader;
        requester & m_req;
        responser & m_res;
        static boost::system::error_code m_protocol_error;
    };
} // namespace api
#endif // API_DISPATCHER_HPP_INCLUDED
