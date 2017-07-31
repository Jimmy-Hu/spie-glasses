#include <json.hpp>
#include <iostream>

using json = nlohmann::json;

int main()
{
    try {
    std::string s;
    while (true) {
        std::getline(std::cin,s);
        if (std::cin.eof()) break;
        json j = json::parse(s);
        auto v = json::to_msgpack(j);
        std::cout.write(reinterpret_cast<char*>(v.data()),v.size());
        std::cout.flush();
    }
    }
    catch (...) {
    }
}
