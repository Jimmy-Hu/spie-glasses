//
// Copyright (C) 2017 CheinYu Lin. All rights reserved.
//
#ifndef API_REQUESTER_HPP_INCLUDED
#define API_REQUESTER_HPP_INCLUDED

// internal api
#include <api/msgpack_writer.hpp>
// external api
#include <msgpack.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
// standard api
#include <memory>
#include <utility>
#include <functional>

namespace api {

    class requester
    {
    public:
        using pair_type = std::pair<std::uint64_t,std::function<void(boost::system::error_code const &, msgpack::object_handle)>>;
        requester(msgpack_queued_writer & qw) : m_id(1), m_qw(qw) {}
        template<typename TupleT, typename HandlerT>
        void request(std::string const & s, TupleT const & t, HandlerT h) {
            auto sp = std::make_shared<msgpack::sbuffer>();
            msgpack::packer<msgpack::sbuffer> pac(*sp);
            pac.pack_array(3);
            pac.pack_fix_uint8(1);
            pac.pack_fix_uint64(m_id);
            pac.pack_array(2);
            pac.pack(s);
            pac.pack(t);
            m_qw.async_write(
                *sp,
                [this,sp,hh=std::move(h),id = m_id] (boost::system::error_code const & ec){
                    if (ec) {
                        std::cerr << "[info] error while requesting id# " << id << std::endl;
                        return;
                    }
                    auto p = m_map.insert(pair_type(id, std::move(hh)));
                    std::cerr << "[info] request done id#" << id << ", set_handler=" << p.second << std::endl;
                });
            ++m_id;
        }
        template<typename TupleT>
        void post(std::string const & s, TupleT const & t) {
            auto sp = std::make_shared<msgpack::sbuffer>();
            msgpack::packer<msgpack::sbuffer> pac(*sp);
            pac.pack_array(3);
            pac.pack_fix_uint8(3);
            pac.pack_fix_uint64(m_id);
            pac.pack_array(2);
            pac.pack(s);
            pac.pack(t);
            m_qw.async_write(
                *sp,
                [this,sp,id = m_id] (boost::system::error_code const & ec){
                    if (ec) {
                        std::cerr << "[info] error while posting id# " << id << std::endl;
                        return;
                    }
                    std::cerr << "[info] post done id#" << id << std::endl;
                });
            ++m_id;
        }
        template<typename TupleT, typename HandlerT>
        void post(std::string const & s, TupleT const & t, HandlerT h) {
            auto sp = std::make_shared<msgpack::sbuffer>();
            msgpack::packer<msgpack::sbuffer> pac(*sp);
            pac.pack_array(3);
            pac.pack_fix_uint8(3);
            pac.pack_fix_uint64(m_id);
            pac.pack_array(2);
            pac.pack(s);
            pac.pack(t);
            m_qw.async_write(
                *sp,
                [this,sp,id = m_id, hh=std::move(h)] (boost::system::error_code const & ec){
                    if (ec) {
                        std::cerr << "[info] error while posting id# " << id << std::endl;
                    } else {
                        std::cerr << "[info] post done id#" << id << std::endl;
                    }
                    hh(ec);
                });
            ++m_id;
        }
        template<typename HandlerT>
        void request_raw(std::string const & s, std::vector<std::uint8_t> const & t, HandlerT h) {
            auto sp = std::make_shared<msgpack::sbuffer>();
            msgpack::packer<msgpack::sbuffer> pac(*sp);
            pac.pack_array(3);
            pac.pack_fix_uint8(1);
            pac.pack_fix_uint64(m_id);
            pac.pack_array(2);
            pac.pack(s);
            sp->write(reinterpret_cast<char const *>(t.data()), t.size());
            m_qw.async_write(
                *sp,
                [this,sp,hh=std::move(h),id = m_id] (boost::system::error_code const & ec){
                    if (ec) {
                        std::cerr << "[info] error while requesting id# " << id << std::endl;
                        this->close(ec);
                        return;
                    }
                    auto p = m_map.insert(pair_type(id, std::move(hh)));
                    std::cerr << "[info] request done id#" << id << ", set_handler=" << p.second << std::endl;
                });
            ++m_id;
        }
        void post_raw(std::string const & s, std::vector<std::uint8_t> const & t) {
            auto sp = std::make_shared<msgpack::sbuffer>();
            msgpack::packer<msgpack::sbuffer> pac(*sp);
            pac.pack_array(3);
            pac.pack_fix_uint8(2);
            pac.pack_fix_uint64(m_id);
            pac.pack_array(2);
            pac.pack(s);
            sp->write(reinterpret_cast<char const *>(t.data()), t.size());
            m_qw.async_write(
                *sp,
                [this,sp,id = m_id] (boost::system::error_code const & ec){
                    if (ec) {
                        std::cerr << "[info] error while post id# " << id << std::endl;
                        this->close(ec);
                        return;
                    }
                    std::cerr << "[info] post done id#" << id << std::endl;
                });
            ++m_id;
        }
        void handle_response(boost::system::error_code const & ec, msgpack::object_handle oh){
            if (ec) {
                std::cerr << "[info] error while handling response" << std::endl;
                close(ec);
                return;
            }
            std::uint64_t id;
            auto o = oh.get();
            o.via.array.ptr[1].convert(id);
            o = o.via.array.ptr[2];
            oh.set(o);
            std::cerr << "[info] response id#" << id << std::endl;
            auto i = m_map.find(id);
            if (i == m_map.end()) {
                std::cerr << "[warning] response mismatch" << std::endl;
                return;
            }
            i->second(ec,std::move(oh));
            m_map.erase(i);
        };
        void close(boost::system::error_code const & ec) {
            for (auto & p : m_map) {
                (p.second)(ec,msgpack::object_handle());
            }
            m_map.clear();
        }
    private:
        std::uint64_t m_id;
        std::map<std::uint64_t,pair_type::second_type> m_map;
        msgpack_queued_writer & m_qw;
    };
} // namespace api
#endif // API_REQUESTER_HPP_INCLUDED
