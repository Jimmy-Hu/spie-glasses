//
// Copyright (C) 2017 CheinYu Lin. All rights reserved.
//
#ifndef API_BASIC_MSGPACK_READER_HPP_INCLUDED
#define API_BASIC_MSGPACK_READER_HPP_INCLUDED

// internal api
#include <api/asio_msgpack.hpp>
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

    template <typename ReadStreamT>
    class basic_msgpack_reader
    {
    public:
        using read_stream_type = ReadStreamT;
        basic_msgpack_reader(read_stream_type rs)
            : m_rs(std::forward<read_stream_type>(rs)) {}
        auto get_executor() {
            return m_rs.get_executor();
        }
        boost::asio::io_context & get_io_context() {
            return static_cast<boost::asio::io_context &>(m_rs.get_executor().context());
        }
        boost::asio::io_context const & get_io_context() const {
            return static_cast<boost::asio::io_context const &>(m_rs.get_executor().context());
        }
        boost::asio::io_context & get_io_service() {
            return get_io_context();
        }
        boost::asio::io_context const & get_io_service() const {
            return get_io_context();
        }
        template <typename ValueT ,typename ReadHandlerT>
        void async_read(ValueT & value, ReadHandlerT rh)
            {
                async_read_msgpack(
                    m_rs,
                    m_unp,
                    value,
                    [rh_=std::move(rh)]
                    (boost::system::error_code const & ec, size_t /*len*/)
                    mutable
                    {
                        rh_(ec);
                    });
            }
    private:
        msgpack::unpacker m_unp;
        read_stream_type m_rs;
    };
} // namespace api
#endif // API_BASIC_MSGPACK_READER_HPP_INCLUDED
