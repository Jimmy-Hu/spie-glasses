#include <msgpack.hpp>
#include <iostream>

int main()
{
    msgpack

    return 0;
}
namespace api {
    template <typename ReadValueT, typename WriteValueT>
    class basic_async_buffer
    {
        using read_value_type = ReadValueT;
        using read_value_wrapped_type = std::conditional_t<
            std::is_reference_t<read_value_type>,
            std::reference_wrapper_t<std::remove_reference_t<read_value_type>>,
            read_value_type>;
        using read_pair_type  = std::pair<read_value_wrapped_type,read_handler_type>;
        using read_queue_type = std::deque<read_pair_type>;

        using write_value_type = WriteValueT;
        using write_value_wrapped_type = std::conditional_t<
            std::is_reference_t<write_value_type>,
            std::reference_wrapper_t<std::remove_reference_t<write_value_type>>,
            write_value_type>;
        using write_pair_type  = std::pair<write_value_wrapped_type,write_handler_type>;
        using write_queue_type = std::deque<write_pair_type>;

        read_queue_type m_rq;
        write_queue_type m_rq;

        void async_read(read_value_type rv, read_handler_type rh);
        void async_write(write_value_type wv, write_handler_type wh) {
             
        }
        void do_run() {
            auto & rp /*read_pair*/     = m_rq.front();
            auto & rv /*read_value*/    = static_cast<read_value_type&>(rp.first);
            auto & rh /*read_handler*/  = rp.second;
            auto & wp /*write_pair*/    = m_wq.front();
            auto & wv /*write_value*/   = static_cast<write_value_type&>(wp.first);
            auto & wh /*write_handler*/ = wp.second;
            rv = std::forward<write_value_type>(wv);
            rh(boost::asio::error_code());
            wh(boost::asio::error_code());
        }
        bool is_io_stopped() {
            return m_rq.empty() || m_wq.empty();
        }
    };
} // namespace api
