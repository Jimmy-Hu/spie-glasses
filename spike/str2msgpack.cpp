// #include <json.hpp>
#include <msgpack.hpp>
#include <iostream>

int main() {
    std::size_t constexpr min_size = 512;
    msgpack::unpacker unp;
    std::string s;
    while (std::getline(std::cin, s)) {
        msgpack::sbuffer sbuf;
        msgpack::pack(sbuf, s);
        std::cout.write(sbuf.data(),sbuf.size());
        std::cout.flush();
    }
    return 0;
}
