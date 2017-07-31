#ifndef API_ASIO_MSGPACK_HPP_INCLUDED
#define API_ASIO_MSGPACK_HPP_INCLUDED

// internal api
// stdandard api
#include <iostream>
#include <map>
#include <utility>
#include <memory>
// external api
#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>
#include <msgpack.hpp>

namespace api {
    namespace detail {
        template <
            typename AsyncReadStreamT,
            typename ValueT,
            typename ReadHandlerT>
        void
        do_async_read_msgpack(
            AsyncReadStreamT & s,
            msgpack::unpacker & unp,
            ValueT & val,
            ReadHandlerT handler,
            boost::system::error_code const & ec,
            size_t byte_transfered)
        {
            // std::cerr << "[debug] n=" << byte_transfered << std::endl;
            // std::cerr << "[debug] s.is_open=" << s.is_open() << std::endl;
            if (ec){
                // std::cerr << "[debug] do_async_read_msgpack: error" << std::endl;
                handler(ec,0);
                return;
            }
            msgpack::object_handle oh;
            if (unp.next(oh)) {
                // std::cerr << "[debug] do_async_read_msgpack: get object" << std::endl;
                oh.get().convert(val);
                handler(ec,byte_transfered - unp.nonparsed_size());
                return;
            }
            // std::cerr << "[debug] do_async_read_msgpack: buffer_capacity=" << unp.buffer_capacity() << std::endl;
            if (unp.buffer_capacity() < 512) {
                // std::cerr << "[debug] do_async_read_msgpack: reserve_buffer" << std::endl;
                unp.reserve_buffer(512);
            }
            // std::cerr << "[debug] do_async_read_msgpack: buffer_capacity=" << unp.buffer_capacity() << std::endl;
            s.async_read_some(
                boost::asio::buffer(unp.buffer(), unp.buffer_capacity()),
                [&s,&unp,&val,h=std::move(handler),byte_transfered]
                (boost::system::error_code const & ec, std::size_t n) mutable {
                    unp.buffer_consumed(n);
                    do_async_read_msgpack(s,unp,val,std::move(h),ec,byte_transfered+n);
                });
        }

        template <
            typename AsyncReadStreamT,
            typename ReadHandlerT>
        void
        do_async_read_msgpack(
            AsyncReadStreamT & s,
            msgpack::unpacker & unp,
            msgpack::object_handle & oh,
            ReadHandlerT handler,
            boost::system::error_code const & ec,
            size_t byte_transfered)
        {
            // std::cerr << "[debug] n=" << byte_transfered << std::endl;
            // std::cerr << "[debug] s.is_open=" << s.is_open() << std::endl;
            if (ec){
                // std::cerr << "[debug] do_async_read_msgpack: error" << std::endl;
                handler(ec,0);
            }
            else if (unp.next(oh)) {
                // std::cerr << "[debug] do_async_read_msgpack: get object" << std::endl;
                handler(ec,byte_transfered - unp.nonparsed_size());
            }
            else {
                unp.reserve_buffer(std::max<std::size_t>(unp.buffer_capacity(), 512));
                // std::cerr << "[debug] do_async_read_msgpack: buffer_capacity=" << unp.buffer_capacity() << std::endl;
                s.async_read_some(
                    boost::asio::buffer(unp.buffer(), unp.buffer_capacity()),
                    [&s,&unp,&oh,h=std::move(handler),byte_transfered]
                    (boost::system::error_code const & ec, std::size_t n) mutable {
                        unp.buffer_consumed(n);
                        do_async_read_msgpack(s,unp,oh,std::move(h),ec,byte_transfered+n);
                    });
            }
            return;
        }
    } // namespace detail

    template <
        typename AsyncWriteStreamT,
        typename ValueT,
        typename WriteHandlerT>
    void
    async_write_msgpack(
        AsyncWriteStreamT & s,
        ValueT const & val,
        WriteHandlerT handler)
    {
        s.get_io_service().post(
            [&s,h=std::move(handler),&val] () mutable
            {
                auto sbuf = std::make_shared<msgpack::sbuffer>();
                msgpack::pack(*sbuf, val);
                //std::cerr << "[debug] async_write (with data="
                          // << (void*)sbuf->data() << ", size="
                          // << sbuf->size() << ")" << std::endl;
                boost::asio::async_write(
                    s,
                    boost::asio::buffer(sbuf->data(),sbuf->size()),
                    [sbuf,hh=std::move(h)]
                    (boost::system::error_code const & ec, size_t len)
                    mutable
                    {
                        hh(ec,len);
                    });
            });
    }

    template <
        typename AsyncWriteStreamT,
        typename WriteHandlerT>
    void
    async_write_msgpack(
        AsyncWriteStreamT & s,
        msgpack::object_handle oh_,
        WriteHandlerT handler)
    {
        s.get_io_service().post(
            [&s,h=std::move(handler),oh = std::move(oh_)] () mutable
            {
                auto sbuf = std::make_shared<msgpack::sbuffer>();
                msgpack::pack(*sbuf, oh.get());
                // //std::cerr << "[debug] async_write (with value="
                //           << oh.get() << ", size="
                //           << sbuf->size() << ")" << std::endl;
                boost::asio::async_write(
                    s,
                    boost::asio::buffer(sbuf->data(),sbuf->size()),
                    [sbuf,hh=std::move(h)]
                    (boost::system::error_code const & ec, size_t len)
                    mutable
                    {
                        hh(ec,len);
                    });
            });
    }

    template <
        typename AsyncReadStreamT,
        typename ValueT,
        typename ReadHandlerT>
    void
    async_read_msgpack(
        AsyncReadStreamT & s,
        msgpack::unpacker & unp,
        ValueT & val,
        ReadHandlerT handler)
    {
        s.get_io_service().post(
            [&s,&unp,&val,h=std::move(handler)]() mutable
            {
                detail::do_async_read_msgpack(
                    s,
                    unp,
                    val,
                    std::move(h),
                    boost::system::error_code(),
                    0);
            });
    }

} // namespace api

#endif // API_ASIO_MSGPACK_HPP_INCLUDED
