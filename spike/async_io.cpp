#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <cstring>
#include <fmt/format.h>

namespace bp = boost::process;
int a1();
int a2();
int a3();

int main (int argc, char** argv) {
    fmt::print("argc={}",argc);
    for (int i = 0; i < argc; ++i) {
        fmt::print("argv[{}]={}\n", i, argv[i]);
    }
    if (argc > 1) {
        if (std::strcmp(argv[1],"1") == 0) return a1();
        else if (std::strcmp(argv[1],"2") == 0) return a2();
        else if (std::strcmp(argv[1],"3") == 0) return a3();
    }
    return a1();
}

int a1()
{
    fmt::print("{} called\n", __func__);
    boost::asio::io_service ios;
    std::vector<char> buf;

    bp::async_pipe ap(ios);

    bp::child c(bp::search_path("cmake"), "--help-command-list", bp::std_out > ap);

    auto on_read =
        [&buf](const boost::system::error_code &ec, std::size_t size){
        fmt::print("on_read()\n");
        fmt::print("  error={}\n", ec.message());
        fmt::print("  size={}\n", size);
    };
    boost::asio::async_read(ap, boost::asio::buffer(buf), on_read);
    ios.run();
    c.wait();
    int result = c.exit_code();
}

int a2()
{
    fmt::print("{} called\n", __func__);
    boost::asio::io_service ios;
    std::vector<char> buf;

    bp::child c(bp::search_path("cmake"), "--help-command-list", bp::std_out > boost::asio::buffer(buf), ios);

    ios.run();
    c.wait();
    int result = c.exit_code();
}

int a3()
{
    fmt::print("{} called\n", __func__);
    boost::asio::io_service ios;

    std::future<std::string> data;

    bp::child c("cmake", "--help-command-list", //set the input
            bp::std_in.close(),
            bp::std_out > data, //so it can be written without anything
            bp::std_err > bp::null,
            ios);


    ios.run(); //this will actually block until the compiler is finished
    auto out = data.get();
    fmt::print(out);
}
