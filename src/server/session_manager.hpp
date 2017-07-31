#ifndef SRC_SIMPLE_SERVER_SESSION_MANAGER_HPP_INCLUDED
#define SRC_SIMPLE_SERVER_SESSION_MANAGER_HPP_INCLUDED

#include "session_fwd.hpp"
#include <set>
#include <map>
#include <iostream>
#include <api/pointer_compare.hpp>

// using session_manager = std::set<session_ptr,api::uintptr_compare<session>>;

struct room {
    session_wptr hint_client;
    session_wptr audio_client;
    session_wptr image_client;
};

using room_ptr = std::shared_ptr<room>;
using room_sptr = std::shared_ptr<room>;
using room_wptr = std::weak_ptr<room>;

struct session_manager : std::map<session_ptr,room_ptr,api::uintptr_compare<session>>
{
    std::map<std::size_t,room_wptr> rooms;
    void start(session_ptr sp) {
        (*this).insert(std::make_pair(sp,room_ptr()));
    }
    void remove(session_ptr sp) {
        (*this).erase(sp);
    }
    // void close() {
    //     for (auto kv : *this) {
    //         // kv.first->close();
    //     }
    //     this->clear();
    // }
    void login(session_ptr sp, std::string s, std::size_t id) {
        auto rsp = rooms[id].lock();
        if (!rsp) {
            std::cerr << "[info] create new room" << std::endl;
            rsp = std::make_shared<room>();
        }
        if (s == "hint-client") {
            rsp->hint_client = sp;
        } else if (s == "audio-client") {
            rsp->audio_client = sp;
        } else if (s == "image-client") {
            rsp->image_client = sp;
        } else {
            std::cerr << "[info] login error" << std::endl;
            return;
        }
        (*this)[sp] = rsp;
        rooms[id] = rsp;
        std::cerr << "[info] client #" << sp <<  " login as " << s << std::endl;
        std::cerr << "[info] hint-client #" << rsp->hint_client.lock() << std::endl;
        std::cerr << "[info] audio-client #" << rsp->audio_client.lock() << std::endl;
        std::cerr << "[info] image-client #" << rsp->image_client.lock() << std::endl;
    }
    room_ptr get_room(session_ptr sp) {
        return (*this)[sp];
    }
};

#endif // SRC_SIMPLE_SERVER_SESSION_MANAGER_HPP_INCLUDED
