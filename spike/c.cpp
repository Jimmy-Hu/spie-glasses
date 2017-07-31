// #include <json.hpp>

// using json = nlohmann::json;


// int main()
// {
//     // create a JSON value
//     json j = R"({"firstName":"John","lastName":"Smith","sex":"male","age":25,"address":{"streetAddress":"212ndStreet","city":"NewYork","state":"NY","postalCode":"10021"},"phoneNumber":[{"type":"home","number":"212555-1234"},{"type":"fax","number":"646555-4567"}]})"_json;

//     // serialize it to MessagePack
//     std::vector<uint8_t> v = json::to_msgpack(j);

//     // print the vector content
//     for (auto& byte : v)
//     {
//         std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
//     }
//     std::cout << std::endl;
//     // deserialize it with MessagePack
//     j = json::from_msgpack(v);

//     // print the deserialized JSON value
//     std::cout << std::setw(2) << j << std::endl;
// }
#include <msgpack.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <utility>
#include <fmt/format.h>

void foo()
{
    // serializes this object.
    std::vector<std::string> vec;
    vec.push_back("Hello");
    vec.push_back("MessagePack");

    // serialize it into simple buffer.
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, vec);

    // deserialize it.
    msgpack::object_handle oh =
        msgpack::unpack(sbuf.data(), sbuf.size());

    // print the deserialized object.
    msgpack::object obj = oh.get();
    msgpack::pack(sbuf, obj);

    oh = msgpack::unpack(sbuf.data(), sbuf.size());
    obj = oh.get();
    std::cout << obj << std::endl;  //=> ["Hello", "MessagePack"]

    // convert it into statically typed object.
    // std::vector<std::string> rvec;
    // obj.convert(rvec);
}

void foo2()
{
    // serializes this object.
    std::vector<std::string> vec;
    vec.push_back("H");
    vec.push_back("MessagePack");
    vec.push_back("adfa jdlasjf ");
    vec.push_back("asdfdf asdf asdf ");

    // serialize it into simple buffer.
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, vec);
    msgpack::pack(sbuf, vec);
    msgpack::pack(sbuf, vec);

    // deserialize it.
    msgpack::unpacker unp(nullptr, nullptr, 0);
    char const * data = sbuf.data();
    int rest = sbuf.size();
    int size = rest < 10 ? rest : 10;
    char const * base = data;
    fmt::print("{}, {}, {}, {}, {}\n",
               data - base,
               unp.buffer_capacity(),
               unp.message_size(),
               unp.parsed_size(),
               unp.nonparsed_size());
    while (rest > 0) {
        unp.reserve_buffer(size);
        std::copy_n(data, size, unp.buffer());
        unp.buffer_consumed(size);
        msgpack::object_handle oh;
        while (unp.next(oh)) {
            // print the deserialized object.
            msgpack::object obj = oh.get();
            std::cout << obj << std::endl;  //=> ["Hello", "MessagePack"]

            // convert it into statically typed object.
            std::vector<std::string> rvec;
            obj.convert(rvec);
        }
        data += size;
        rest -= size;
        size = rest < 10 ? rest : 10;
        fmt::print("{}, {}, {}, {}, {}\n",
                   data - base,
                   unp.buffer_capacity(),
                   unp.message_size(),
                   unp.parsed_size(),
                   unp.nonparsed_size());
    }
}

int main(void)
{
    foo();
    // msgpack::unpacker unp(nullptr,nullptr,0);
    // msgpack::unpacker unp2(unp);
    // msgpack::object_handle oh1;
    // msgpack::object_handle oh2();//(std::move(oh1));
    // oh2 = std::move(oh1);
    // oh2 = oh1;
    // std::cout << unp.buffer_capacity() << std::endl;
}
