#include <json.hpp>
#include <iostream>

using json = nlohmann::json;

int main()
{
    try {
        json j;
        while (true) {
            std::cin >> j;
            auto v = json::to_msgpack(j);
            std::cout.write(reinterpret_cast<char*>(v.data()),v.size());
            std::cout.flush();
            if (std::cin.eof()) break;
        }
    }
    catch (std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
}
