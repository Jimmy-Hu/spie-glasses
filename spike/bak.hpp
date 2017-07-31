    // template <
    //     typename AsyncReadStreamT,
    //     typename MessageT,
    //     typename ReadHandlerT>
    // void
    // message_reader::async_read_msgpack(
    //     AsyncReadStreamT & s,
    //     msgpack::unpacker & unp,
    //     msgpack::object_handle & oh,
    //     ReadHandlerT handler)
    // {
    //     s.get_io_service().post(
    //         [&s,&unp,&oh,handler=std::move(handler)]
    //         (){
    //             detail::do_async_read_msgpack(
    //                 s,
    //                 unp,
    //                 oh,
    //                 std::move(handler),
    //                 boost::system::error_code(),
    //                 unp.message_size());
    //         });
    // }

    // template <
    //     typename AsyncWriteStreamT,
    //     typename MessageT,
    //     typename WriteHandlerT>
    // void
    // async_write_msgpack(
    //     AsyncWriteStreamT const & s,
    //     msgpack::object_handle const & oh,
    //     WriteHandlerT handler)
    // {
    //     s.get_io_service().post(
    //         [&s,&oh,handler=std::move(handler)]
    //         (){
    //             std::unique_ptr<msgpack::sbuffer> sbuf;
    //             msgpack::pack(*sbuf, oh.get());
    //             boost::asio::async_write(
    //                 s,
    //                 boost::asio::buffer(sbuf.data(),sbuf.size()),
    //                 [sbuf=std::move(sbuf),handler=std::move(handler)]
    //                 (boost::system::error_code ec, size_t len)
    //                 {
    //                     handler(ec,len);
    //                 });
    //         });
    // }

    // template <
    //     typename AsyncWriteStreamT,
    //     typename MessageT,
    //     typename WriteHandlerT>
    // void
    // async_write_msgpack(
    //     AsyncWriteStreamT const & s,
    //     msgpack::object_handle const & oh,
    //     WriteHandlerT handler)
    // {
    //     s.get_io_service().post(
    //         [&s,&oh,handler=std::move(handler)]
    //         (){
    //             std::unique_ptr<msgpack::sbuffer> sbuf;
    //             msgpack::pack(*sbuf, oh.get());
    //             boost::asio::async_write(
    //                 s,
    //                 boost::asio::buffer(sbuf.data(),sbuf.size()),
    //                 [sbuf=std::move(sbuf),handler=std::move(handler)]
    //                 (boost::system::error_code ec, size_t len)
    //                 {
    //                     handler(ec,len);
    //                 });
    //         });
    // }
    // template <typename AsyncReadStreamT>
    // class basic_msgpack_reader
    // {
    //     basic_msgpack_reader(AsyncReadStreamT & s)
    //         : m_s(s), m_unp() {}
    //     AsyncReadStreamT m_s;
    //     msgpack::unpacker m_unp;
    //     template <typename ReadHandlerT>
    //     void async_read(msgpack::object_handle & oh, ReadHandlerT h)
    //     {
    //         async_read_msgpack(m_s, m_unp, oh, std::move(h));
    //     }
    //     template <typename ReadHandlerT>
    //     void async_get(msgpack::object_handle & oh, ReadHandlerT h)
    //     {
    //         async_read_msgpack(m_s, m_unp, oh, std::move(h));
    //     }
    // };
    // template <typename AsyncWriteStreamT>
    // class basic_msgpack_writer
    // {
    // public:
    //     using value_type = msgpack::object_handle;
    //     basic_msgpack_writer(AsyncWriteStreamT & s)
    //         : m_s(s) {}
    //     template <typename WriteHandlerT>
    //     void async_write(msgpack::object_handle & oh, WriteHandlerT h)
    //     {
    //         async_write_msgpack(m_s, m_unp, oh, std::move(h));
    //     }
    // private:
    //     AsyncWriteStreamT m_s;
    // };
    // template <typename ReaderT,
    //           typename ReadValueT,
    //           typename ReadHandlerSignatureT>
    // class basic_queued_reader
    // {
    // public:
    //     using reader_type            = ReaderT;
    //     using read_value_type             = ValueT;
    //     using read_handler_signature_type = HandlerSignatureT;
    //     using read_handler_type  = std::function<handler_signature_type>;
    //     using read_pair_type     = std::pair<value_type,handler_type>;
    //     using read_queue_type    = std::deque<pair_type>;
    //     basic_queued_reader(ReaderT reader)
    //       : m_reader(static_cast<ReaderT>(reader))
    //       , m_rq()
    //     {}
    //     void async_read(read_value_type v, read_handler_type h) {
    //         m_reader.get_io_service().post(
    //             [this, vv = std::forward<read_value_type>(v), hh = std::forward<read_handler_type>(h)]() mutable {
    //                 bool is_read_stopped = m_rq.empty();
    //                 m_rq.push_back(pair_type(std::forward<read_value_type>(v),std::forward<read_handler_type>(h)));
    //                 do_read();
    //             });
    //     }
    // private:
    //     void do_read() {
    //         m_reader.async_read(std::forward<read_value_type>(v),std::forward<read_handler_type>(h));
    //         if (!m_rq.empty())
    //             m_reader.get_io_service().post(std::bind(do_read,this));
    //     }
    //     reader_type m_reader;
    //     read_queue_type m_rq;
    // };

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
