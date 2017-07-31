#include <json.hpp>
#include <msgpack.hpp>
#include <cstdio>

int main() {
    msgpack::unpacker unp;
    msgpack::object_handle oh;
    while (true) {
        unp.reserve_buffer(4096);
        // std::cin.get(*unp.buffer());
        std::size_t n = std::fread(unp.buffer(), 1, 4096, stdin);
        unp.buffer_consumed(n);
        while (unp.next(oh)) {
            std::cout << oh.get() << std::endl;
        }
        if (std::feof(stdin)) {
            break;
        }
    }
    return 0;
}
// std::size_t constexpr min_size = 512;
// std::size_t n;
// n = std::cin.readsome(unp.buffer() + 1, unp.buffer_capacity() - 1) + 1;
// std::cerr << n << std::endl;
// if (n == 0) {
//     std::cerr << std::cin.gcount() << std::endl;
//     std::cerr << "[debug] error get input characters" << std::endl;
//     return 0;
// }
