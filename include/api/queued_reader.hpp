#ifndef API_QUEUED_READER_HPP_INCLUDED
#define API_QUEUED_READER_HPP_INCLUDED

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
} // namespace api
#endif // API_QUEUED_READER_HPP_INCLUDED
