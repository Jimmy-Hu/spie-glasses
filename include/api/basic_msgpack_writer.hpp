//
// Copyright (C) 2017 CheinYu Lin. All rights reserved.
//
#ifndef API_BASIC_MSGPACK_WRITER_HPP_INCLUDED
#define API_BASIC_MSGPACK_WRITER_HPP_INCLUDED

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

    template <typename WriteStreamT>
    class basic_msgpack_writer
    {
    public:
        using write_stream_type = WriteStreamT;
        basic_msgpack_writer(write_stream_type ws)
            : m_ws(std::forward<write_stream_type>(ws)) {}
        boost::asio::io_service & get_io_service() {
            return m_ws.get_io_service();
        }
        boost::asio::io_service const & get_io_service() const {
            return m_ws.get_io_service();
        }
        template <typename BufferT, typename WriteHandlerT>
        void async_write(BufferT const & buf, WriteHandlerT wh)
            {
                boost::asio::async_write(
                    m_ws,
                    boost::asio::buffer(buf.data(), buf.size()),
                    [wh_=std::move(wh)]
                    (boost::system::error_code const & ec, size_t /*len*/)
                    mutable
                    {
                        wh_(ec);
                    });
            }
    private:
        write_stream_type m_ws;
    };

} // namespace api
#endif // API_BASIC_MSGPACK_WRITER_HPP_INCLUDED
