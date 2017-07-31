#include <api/parser.hpp>
#include <msgpack.hpp>

#include <string>
#include <iostream>

int main() {
    std::string s;
    bool continued = false;
    api::command_parser cp;
    while (true) {
        // if (!continued)
        //     std::cout << "> ";
        if (!std::getline(std::cin,s))
            return 0;
        cp.parse(s.begin(),s.end());
        if (cp.is_complete()) {
            msgpack::sbuffer sbuf;
            msgpack::pack(sbuf, cp.get());
            std::cout.write(sbuf.data(),sbuf.size());
            std::cout.flush();
            cp.clear();
            continued = false;
        }
        else {
            continued = true;
        }
    }
}
