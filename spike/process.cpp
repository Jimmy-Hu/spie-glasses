#include <boost/version.hpp>
#if BOOST_VERSION >= 108800
#include <boost/process/v1.hpp>
using namespace boost::process::v1;
#else
#include <boost/process.hpp>
using namespace boost::process;
#endif
#include <iostream>

int main()
{
    ipstream pipe_stream;
    child c("cmake --help-command-list", std_out > pipe_stream);

    std::string line;

    while (pipe_stream && std::getline(pipe_stream, line) && !line.empty())
        std::cerr << line << std::endl;

    c.wait();
}
