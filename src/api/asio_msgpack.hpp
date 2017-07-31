#ifndef API_ASIO_MSGPACK_HPP_INCLUDED
#define API_ASIO_MSGPACK_HPP_INCLUDED
#include <boost/asio.hpp>
#include <iostream>
#include <utility>
#include <memory>

namespace api {
    namespace detail {
        template <
            typename AsyncReadStreamT,
            typename MessageT,
            typename ReadHandlerT>
        void
        do_async_read_msgpack(
            AsyncReadStreamT & s,
            msgpack::unpacker & unp,
            ValueT & val,
            ReadHandlerT handler,
            boost::system::error_code ec,
            size_t byte_transfered)
        {
            if (ec){
                handler(ec,0);
                return;
            }
            msgpack::object_handle oh;
            if (unp.next(oh)) {
                oh.get().convert(val);
                handler(ec,byte_transfered - unp.nonparsed_size());
            }
            else {
                unp.reserve_buffer(std::max(unp.buffer_capacity(), 512));
                s.async_read_some(
                    boost::asio::buffer(unp.buffer(), unp.buffer_capacity()),
                    [&s,&unp,&val,h=std::move(handler),ec,byte_transfered]
                    (boost::system::error_code ec, std::size_t n) {
                        unp.buffer_consumed(n);
                        do_async_read_msgpack(s,unp,val,std::move(h),ec,byte_transfered+n);
                    });
            }
            return;
        }
        template <
            typename AsyncReadStreamT,
            typename MessageT,
            typename ValueT,
            typename ReadHandlerT>
        do_async_read_msgpack(
            AsyncReadStreamT & s,
            msgpack::unpacker & unp,
            ReadHandlerT & handler,
            boost::system::error_code ec,
            size_t byte_transfered)
        {
            if (ec){
                handler(ec,0);
            }
            else if (unp.next(oh)) {
                handler(ec,byte_transfered - unp.nonparsed_size());
            }
            else {
                unp.reserve_buffer(std::max(unp.buffer_capacity(), 512));
                s.async_read_some(
                    boost::asio::buffer(unp.buffer(), unp.buffer_capacity()),
                    [&s,&unp,&oh,h=std::move(handler),ec,byte_transfered]
                    (boost::system::error_code ec, std::size_t n) {
                        unp.buffer_consumed(n);
                        do_async_read_msgpack(s,unp,oh,std::move(h),ec,byte_transfered+n);
                    });
            }
            return;
        }
    } // namespace detail
    template <
        typename AsyncReadStreamT,
        typename MessageT,
        typename ReadHandlerT>
    void
    message_reader::async_read_msgpack(
        AsyncReadStreamT & s,
        msgpack::unpacker & unp,
        msgpack::object_handle & oh,
        ReadHandlerT handler)
    {
        s.get_io_service().post(
            [&s,&unp,&oh,handler=std::move(handler)]
            (){
                detail::do_async_read_msgpack(
                    s,
                    unp,
                    oh,
                    std::move(handler),
                    boost::system::error_code(),
                    unp.message_size());
            });
    }

    template <
        typename AsyncWriteStreamT,
        typename MessageT,
        typename WriteHandlerT>
    void
    async_write_msgpack(
        AsyncWriteStreamT const & s,
        msgpack::object_handle const & oh,
        WriteHandlerT handler)
    {
        s.get_io_service().post(
            [&s,&oh,handler=std::move(handler)]
            (){
                std::unique_ptr<msgpack::sbuffer> sbuf;
                msgpack::pack(*sbuf, oh.get());
                boost::asio::async_write(
                    s,
                    boost::asio::buffer(sbuf.data(),sbuf.size()),
                    [sbuf=std::move(sbuf),handler=std::move(handler)]
                    (boost::system::error_code ec, size_t len)
                    {
                        handler(ec,len);
                    });
            });

    }
    template <
        typename AsyncWriteStreamT,
        typename MessageT,
        typename WriteHandlerT>
    void
    async_write_msgpack(
        AsyncWriteStreamT const & s,
        msgpack::object_handle const & oh,
        WriteHandlerT handler)
    {
        s.get_io_service().post(
            [&s,&oh,handler=std::move(handler)]
            (){
                std::unique_ptr<msgpack::sbuffer> sbuf;
                msgpack::pack(*sbuf, oh.get());
                boost::asio::async_write(
                    s,
                    boost::asio::buffer(sbuf.data(),sbuf.size()),
                    [sbuf=std::move(sbuf),handler=std::move(handler)]
                    (boost::system::error_code ec, size_t len)
                    {
                        handler(ec,len);
                    });
            });
    }
    template <
        typename AsyncWriteStreamT,
        typename MessageT,
        typename ValueT,
        typename WriteHandlerT>
    void
    async_write_msgpack(
        AsyncWriteStreamT const & s,
        ValueT const & val,
        WriteHandlerT handler)
    {
        s.get_io_service().post(
            [&s,&oh,handler=std::move(handler)]
            (){
                std::unique_ptr<msgpack::sbuffer> sbuf;
                msgpack::pack(*sbuf, val);
                boost::asio::async_write(
                    s,
                    boost::asio::buffer(sbuf.data(),sbuf.size()),
                    [sbuf=std::move(sbuf),handler=std::move(handler)]
                    (boost::system::error_code const &ec, size_t len)
                    {
                        handler(ec,len);
                    });
            });
    }
} // namespace api

#endif // API_ASIO_MSGPACK_HPP_INCLUDED
