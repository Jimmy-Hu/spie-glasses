//
// Copyright (C) 2017 CheinYu Lin. All rights reserved.
//
#ifndef API_QUEUED_WRITER_HPP_INCLUDED
#define API_QUEUED_WRITER_HPP_INCLUDED

// internal api
// external api
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
// standard api
#include <memory>
#include <utility>
#include <deque>
#include <functional>

namespace api {
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
} // namespace api
#endif // API_QUEUED_WRITER_HPP_INCLUDED
