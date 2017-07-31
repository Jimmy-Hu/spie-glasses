#include <cstdio>
#include <cstdint>
#include <iostream>

int main() {
    std::uint8_t buf[1024] = {0};
    std::FILE * f = std::fopen("f.txt", "rb");
    std::size_t n = std::fread(buf, 4, 2, f);
    std::cout << buf << std::endl;
    std::cout << "read_count: " << n << std::endl;
    std::cout << "ferror: " << std::ferror(f) << std::endl;
    std::cout << "feof: " << std::feof(f) << std::endl;
}
