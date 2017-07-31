#ifndef API_MSGPACK_WRITER_HPP_INCLUDED
#define API_MSGPACK_WRITER_HPP_INCLUDED

// internal api
#include <api/basic_msgpack_writer.hpp>
#include <api/queued_writer.hpp>
// external api
#include <boost/system/error_code.hpp>
#include <msgpack.hpp>
// stdandard api


namespace api {
    using msgpack_writer          = basic_msgpack_writer<boost::asio::ip::tcp::socket &>;
    using msgpack_queued_writer   = basic_queued_writer<msgpack_writer,msgpack::sbuffer,void(boost::system::error_code const &)>;
} // namespace api
#endif // API_MSGPACK_WRITER_HPP_INCLUDED
