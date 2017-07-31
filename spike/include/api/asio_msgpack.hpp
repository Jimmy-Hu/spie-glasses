#ifndef API_ASIO_MSGPACK_HPP_INCLUDED
#define API_ASIO_MSGPACK_HPP_INCLUDED
#include <boost/asio.hpp>
#include <map>
#include <msgpack.hpp>
#include <iostream>
#include <utility>
#include <memory>
#include <api/functional.hpp>

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
            //std::cerr << "[debug] n=" << byte_transfered << std::endl;
            if (ec){
                //std::cerr << "[debug] do_async_read_msgpack: error" << std::endl;
                handler(ec,0);
                return;
            }
            msgpack::object_handle oh;
            if (unp.next(oh)) {
                //std::cerr << "[debug] do_async_read_msgpack: get object" << std::endl;
                oh.get().convert(val);
                handler(ec,byte_transfered - unp.nonparsed_size());
                return;
            }
            //std::cerr << "[debug] do_async_read_msgpack: buffer_capacity=" << unp.buffer_capacity() << std::endl;
            if (unp.buffer_capacity() < 512) {
                //std::cerr << "[debug] do_async_read_msgpack: reserve_buffer" << std::endl;
                unp.reserve_buffer(512);
            }
            //std::cerr << "[debug] do_async_read_msgpack: buffer_capacity=" << unp.buffer_capacity() << std::endl;
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
            if (ec){
                handler(ec,0);
            }
            else if (unp.next(oh)) {
                handler(ec,byte_transfered - unp.nonparsed_size());
            }
            else {
                unp.reserve_buffer(std::max<std::size_t>(unp.buffer_capacity(), 512));
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

    template <typename WriteStreamT>
    class basic_buffer_writer
    {
    public:
        using write_stream_type = WriteStreamT;
        basic_buffer_writer(write_stream_type ws)
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

    template <typename WriterT,
              typename WriteValueT,
              typename WriteHandlerSignatureT>
    class basic_queued_writer
    {
    public:
        using writer_type                  = WriterT;
        using write_value_type             = WriteValueT;
        using write_wrapped_value_type     = std::reference_wrapper<WriteValueT const>;
        using write_handler_signature_type = WriteHandlerSignatureT;
        using write_handler_type  = std::function<write_handler_signature_type>;
        using write_pair_type     = std::pair<write_wrapped_value_type,write_handler_type>;
        using write_queue_type    = std::deque<write_pair_type>;
        basic_queued_writer(WriterT writer)
            : m_writer(std::forward<WriterT>(writer))
            , m_wq()
            {}
        boost::asio::io_service & get_io_service() {
            return m_writer.get_io_service();
        }
        boost::asio::io_service const & get_io_service() const {
            return m_writer.get_io_service();
        }
        void async_write(write_value_type const & v, write_handler_type h) {
            m_writer.get_io_service().post(
                [this, &v, hh = std::move(h)]() mutable {
                    bool is_write_stopped = m_wq.empty();
                    m_wq.push_back(write_pair_type(v,std::move(hh)));
                    if (is_write_stopped)
                        do_write();
                });
        }
    private:
        void do_write() {
            auto & wv = m_wq.front().first.get();
            auto & wh = m_wq.front().second;
            m_writer.async_write(wv, std::move(wh));
            m_wq.pop_front();
            if (!m_wq.empty())
                m_writer.get_io_service().post([this](){this->do_write();});
        }
        writer_type m_writer;
        write_queue_type m_wq;
    };

    template <typename ReadStreamT>
    class basic_msgpack_handle_reader
    {
    public:
        using read_stream_type = ReadStreamT;
        basic_msgpack_handle_reader(read_stream_type rs)
            : m_rs(std::forward<read_stream_type>(rs)) {}
        boost::asio::io_service & get_io_service() {
            return m_rs.get_io_service();
        }
        boost::asio::io_service const & get_io_service() const {
            return m_rs.get_io_service();
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

    template <typename ReaderT,
              typename ReadValueT,
              typename ReadHandlerSignatureT>
    class basic_queued_reader
    {
    public:
        using reader_type                 = ReaderT;
        using read_value_type             = ReadValueT;
        using read_wrapped_value_type     = std::reference_wrapper<ReadValueT>;
        using read_handler_signature_type = ReadHandlerSignatureT;
        using read_handler_type  = std::function<read_handler_signature_type>;
        using read_pair_type     = std::pair<read_wrapped_value_type,read_handler_type>;
        using read_queue_type    = std::deque<read_pair_type>;
        basic_queued_reader(ReaderT reader)
            : m_reader(std::forward<ReaderT>(reader))
            , m_rq()
            {}
        boost::asio::io_service & get_io_service() {
            return m_reader.get_io_service();
        }
        boost::asio::io_service const & get_io_service() const {
            return m_reader.get_io_service();
        }
        void async_read(read_value_type & v, read_handler_type h) {
            m_reader.get_io_service().post(
                [this, &v, hh = std::move(h)]() mutable {
                    bool is_read_stopped = m_rq.empty();
                    m_rq.push_back(read_pair_type(v,std::move(hh)));
                    if (is_read_stopped)
                        do_read();
                });
        }
    private:
        void do_read() {
            auto & rv = m_rq.front().first.get();
            auto & rh = m_rq.front().second;
            m_reader.async_read(rv, std::move(rh));
            m_rq.pop_front();
            if (!m_rq.empty())
                m_reader.get_io_service().post(
                    [this](){this->do_read();});
        }
        reader_type m_reader;
        read_queue_type m_rq;
    };


    using tcp_socket_buffer_writer = basic_buffer_writer<boost::asio::ip::tcp::socket &>;
    using tcp_socket_queued_sbuffer_writer = basic_queued_writer<tcp_socket_buffer_writer,msgpack::sbuffer,void(boost::system::error_code const &)>;
    using tcp_socket_msgpack_handle_reader = basic_msgpack_handle_reader<boost::asio::ip::tcp::socket &>;
    using tcp_socket_queued_msgpack_handle_reader = basic_queued_reader<tcp_socket_msgpack_handle_reader,msgpack::object_handle,void(boost::system::error_code const &)>;

    class requester
    {
    public:
        using pair_type = std::pair<std::uint64_t,std::function<void(boost::system::error_code const &, msgpack::object_handle)>>;
        requester(tcp_socket_queued_sbuffer_writer & qw) : m_qw(qw), m_id(1) {}
        template<typename TupleT, typename HandlerT>
        void request(std::string const & s, TupleT const & t, HandlerT h) {
            auto sp = std::make_shared<msgpack::sbuffer>();
            msgpack::packer<msgpack::sbuffer> pac(*sp);
            pac.pack_array(3);
            pac.pack_fix_uint8(1);
            pac.pack_fix_uint64(m_id);
            pac.pack_array(2);
            pac.pack(s);
            pac.pack(t);
            m_qw.async_write(
                *sp,
                [this,sp,hh=std::move(h),id = m_id] (boost::system::error_code const & ec){
                    if (ec) {
                        //std::cerr << "[info] error while requesting id# " << id << std::endl;
                        return;
                    }
                    auto p = m_map.insert(pair_type(id, std::move(hh)));
                    //std::cerr << "[info] request done id#" << id << ", set_handler=" << p.second << std::endl;
                });
            ++m_id;
        }
        template<typename HandlerT>
        void request_raw(std::string const & s, std::vector<std::uint8_t> const & t, HandlerT h) {
            auto sp = std::make_shared<msgpack::sbuffer>();
            msgpack::packer<msgpack::sbuffer> pac(*sp);
            pac.pack_array(3);
            pac.pack_fix_uint8(1);
            pac.pack_fix_uint64(m_id);
            pac.pack_array(2);
            pac.pack(s);
            sp->write(reinterpret_cast<char const *>(t.data()), t.size());
            m_qw.async_write(
                *sp,
                [this,sp,hh=std::move(h),id = m_id] (boost::system::error_code const & ec){
                    if (ec) {
                        //std::cerr << "[info] error while requesting id# " << id << std::endl;
                        return;
                    }
                    auto p = m_map.insert(pair_type(id, std::move(hh)));
                    //std::cerr << "[info] request done id#" << id << ", set_handler=" << p.second << std::endl;
                });
            ++m_id;
        }
        void handle_response(boost::system::error_code const & ec, msgpack::object_handle oh){
            if (ec) {
                //std::cerr << "[info] error while handling response" << std::endl;
                for (auto & p : m_map) {
                    (p.second)(ec,msgpack::object_handle());
                }
                m_map.clear();
                return;
            }
            std::uint64_t id;
            auto o = oh.get();
            o.via.array.ptr[1].convert(id);
            o = o.via.array.ptr[2];
            oh.set(o);
            //std::cerr << "[info] response id#" << id << std::endl;
            auto i = m_map.find(id);
            if (i == m_map.end()) {
                //std::cerr << "[warning] response mismatch" << std::endl;
                return;
            }
            i->second(ec,std::move(oh));
            m_map.erase(i);
        };
    private:
        std::uint64_t m_id;
        std::map<std::uint64_t,pair_type::second_type> m_map;
        tcp_socket_queued_sbuffer_writer & m_qw;
    };

    namespace detail {
        template <typename T> struct signature_args_as_tuple;
        template <typename ResultT, typename ...ArgTs>
        struct signature_args_as_tuple<ResultT(ArgTs...)> {
            using type = std::tuple<ArgTs...>;
        };
        template <typename BufferT, typename FunctionT, typename TupleT, typename std::enable_if_t<!std::is_void<decltype(api::apply(std::declval<FunctionT>(),std::declval<TupleT>()))>::value,int(*)[1]> = nullptr>
        void apply_and_pack(BufferT & buf, FunctionT&& f, TupleT && t) {
            msgpack::pack(buf,api::apply(std::forward<FunctionT>(f),std::forward<TupleT>(t)));
        }
        template <typename BufferT, typename FunctionT, typename TupleT, typename std::enable_if_t<std::is_void<decltype(api::apply(std::declval<FunctionT>(),std::declval<TupleT>()))>::value,int(*)[2]> = nullptr>
        void apply_and_pack(BufferT & buf, FunctionT&& f, TupleT && t) {
            api::apply(std::forward<FunctionT>(f),std::forward<TupleT>(t));
            msgpack::pack(buf,msgpack::type::nil_t{});
        }
    }

    class responser
    {
    public:
        using pair_type = std::pair<std::string,
                                    std::function<void(msgpack::object_handle, msgpack::sbuffer &)>>;

        responser(tcp_socket_queued_sbuffer_writer & qw) : m_qw(qw), m_id(1) {}
        void handle_request(boost::system::error_code const & ec, msgpack::object_handle oh){
            if (ec) {
                //std::cerr << "[info] error while handling request" << std::endl;
                return;
            }
            std::uint64_t id;
            std::string str;
            auto o = oh.get();
            o.via.array.ptr[1].convert(id);
            o = o.via.array.ptr[2];
            o.via.array.ptr[0].convert(str);
            o = o.via.array.ptr[1];
            auto i = m_map.find(str);
            //std::cerr << "[info] handle request " << str << std::endl;
            if (i == m_map.end()) {
                //std::cerr << "[warning] no request handler found" << std::endl;
                return;
            }
            oh.set(o);
            std::shared_ptr<msgpack::sbuffer> sp = std::make_shared<msgpack::sbuffer>();
            msgpack::packer<msgpack::sbuffer> pac(*sp);
            pac.pack_array(3);
            pac.pack_fix_uint8(2);
            pac.pack_fix_uint64(id);
            try {
                i->second(std::move(oh),*sp);
            } catch(...) {
                pac.pack_nil();
            }
            m_qw.async_write(
                *sp,
                [this,sp,id] (boost::system::error_code const & ec){
                    if (ec) {
                        //std::cerr << "[info] response error" << std::endl;
                    }
                    else {
                        //std::cerr << "[info] response done id#" << id << std::endl;
                    }
                });
        };

        template <typename SignatureT>
        void set_request_handler(std::string const & s, std::function<SignatureT> f) {
            m_map[s] = [ff = std::move(f)](msgpack::object_handle oh, msgpack::sbuffer & sbuf){
                using tuple_type = typename detail::signature_args_as_tuple<SignatureT>::type;
                tuple_type tup;
                oh.get().convert(tup);
                detail::apply_and_pack(sbuf,ff,tup);
            };
        }
    private:
        std::uint64_t m_id;
        std::map<std::string,pair_type::second_type> m_map;
        tcp_socket_queued_sbuffer_writer & m_qw;
    };
    class dispatcher {
    public:
        dispatcher(tcp_socket_queued_msgpack_handle_reader & reader, requester & req, responser & res)
            : m_reader(reader)
            , m_req(req)
            , m_res(res)
            {}
        void async_dispatch() {
            auto sp = std::make_shared<msgpack::object_handle>();
            m_reader.async_read(*sp, [sp,this](boost::system::error_code const & ec){
                    do_dispatch(ec,std::move(*sp));
                });
        }
        void set_error_handler(std::function<void(boost::system::error_code const &)> fn) {
            m_eh = std::move(fn);
        }
    private:
        void do_dispatch(boost::system::error_code ec, msgpack::object_handle oh) {
            if (!ec) async_dispatch();
            else if (m_eh) {
                //std::cerr << "[info] dispatch error"  << std::endl;
                m_eh(ec);
            }
            auto o = oh.get();
            enum {REQUEST = 1, RESPONSE = 2};
            if (o.type != msgpack::type::ARRAY || o.via.array.size != 3) {
                return;
            }
            std::uint64_t type;
            std::uint64_t id;
            o.via.array.ptr[0].convert(type);
            o.via.array.ptr[1].convert(id);
            //std::cerr << "[info] dispatch object=" << o << std::endl;
            switch (type) {
            case REQUEST:
                m_res.handle_request(ec,std::move(oh));
                break;
            case RESPONSE:
                m_req.handle_response(ec,std::move(oh));
                break;
            default: ;
            }
        }
        std::function<void(boost::system::error_code const &)> m_eh;
        tcp_socket_queued_msgpack_handle_reader & m_reader;
        requester & m_req;
        responser & m_res;
    };
} // namespace api

namespace detail {
}

#endif // API_ASIO_MSGPACK_HPP_INCLUDED
