#ifndef API_MSGPACK_READER_HPP_INCLUDED
#define API_MSGPACK_READER_HPP_INCLUDED

// internal api
#include <api/basic_msgpack_reader.hpp>
#include <api/queued_reader.hpp>
// external api
#include <boost/system/error_code.hpp>
#include <msgpack.hpp>
// stdandard api


namespace api {
    using msgpack_reader          = basic_msgpack_reader<boost::asio::ip::tcp::socket &>;
    using msgpack_queued_reader   = basic_queued_reader<msgpack_reader, msgpack::object_handle,void(boost::system::error_code const &)>;
} // namespace api
#endif // API_MSGPACK_READER_HPP_INCLUDED
