#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <msgpack.hpp>
// #include <deque>
// #include <fmt.hpp>
// #include <json.hpp>
// #include <msgpack.hpp>

// int main() {
//     char a[32];
//     using mb = boost::asio::mutable_buffer;
//     std::vector<boost::asio::mutable_buffer> v{mb(a,0)};
//     boost::asio::detail::consuming_buffers
//     < boost::asio::mutable_buffer
//     , std::vector<boost::asio::mutable_buffer>
//     > c(v);
//     auto & cc = c;
//     std::cout << std::boolalpha << (cc.begin() == cc.end()) << std::endl;
//     return 0;
// }

struct msgpack_reader : msgpack::unpacker
{
    template <typename ...ArgTs> msgpack_reader(ArgTs && ...args);
    template <typename Stream, typename Handler>
    async_read(Stream & s, msgpack::object_handle & oh, Handler handler);

};

template <typename ...ArgTs>
msgpack_reader::msgpack_reader(ArgTs && args)
    : msgpack::unpacker(std::forward<ArgTs>(args)...)
{}

template <typename Stream, typename Handler>
msgpack_reader::async_read(Stream & s, msgpack::object_handle & oh, Handler handler)
{
    s.async_read_some()
}
