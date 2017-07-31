#include <boost/process.hpp>
#include <vector>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <fmt/format.h>

int main() {
    namespace bp = boost::process;
    boost::asio::io_service ios;
    std::vector<char> buf;

    bp::async_pipe ap(ios,stdin);

    // ap < stdin;
    // ap > stdout;

    // bp::child c(bp::search_path("cmake"), "--help", bp::std_out > ap, bp::std_out > stdout);

    auto f = [&ap](const boost::system::error_code &ec, std::size_t size)
        {
            fmt::print(
                "error: {}\n"
                "size : {}\n",
                std::system_category().message(ec.value()),
                size);
        };
    boost::asio::async_read(ap, boost::asio::buffer(buf),f);
    ios.run();
    c.wait();
    int result = c.exit_code();
}
