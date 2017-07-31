#ifndef API_RESPONSER_HPP_INCLUDED
#define API_RESPONSER_HPP_INCLUDED

// internal api
#include <api/asio_msgpack.hpp>
#include <api/functional.hpp>
#include <api/msgpack_reader.hpp>
#include <api/msgpack_writer.hpp>
// external api
#include <msgpack.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
// standard api
#include <memory>
#include <map>
#include <utility>
#include <functional>

namespace api {
    namespace detail {
        template <typename T> struct signature_args_as_tuple;
        template <typename ResultT, typename ...ArgTs>
        struct signature_args_as_tuple<ResultT(ArgTs...)> {
            using type = std::tuple<ArgTs...>;
        };
        template <typename BufferT, typename FunctionT, typename TupleT, typename std::enable_if_t<!std::is_void<decltype(api::apply(std::declval<FunctionT>(),std::declval<TupleT>()))>::value,int(*)[1]> = nullptr>
        void apply_and_pack(BufferT & buf, FunctionT&& f, TupleT && t) {
            msgpack::pack(buf,api::apply(std::forward<FunctionT>(f),std::forward<TupleT>(t)));
        }
        template <typename BufferT, typename FunctionT, typename TupleT, typename std::enable_if_t<std::is_void<decltype(api::apply(std::declval<FunctionT>(),std::declval<TupleT>()))>::value,int(*)[2]> = nullptr>
        void apply_and_pack(BufferT & buf, FunctionT&& f, TupleT && t) {
            api::apply(std::forward<FunctionT>(f),std::forward<TupleT>(t));
            msgpack::pack(buf,msgpack::type::nil_t{});
        }
    }

    class responser
    {
    public:
        using pair_type = std::pair<std::string,
                                    std::function<void(msgpack::object_handle, msgpack::sbuffer &)>>;
        using post_pair_type = std::pair<std::string,
                                    std::function<void(msgpack::object_handle)>>;

        responser(msgpack_queued_writer & qw) : m_qw(qw) {}
        bool assert_(bool cond, char const * msg) {
            if (cond) return cond;
            std::cerr << "[info][dispatcher] assert failure: " << msg << ", closing..."  << std::endl;
            return cond;
        }
        void handle_request(boost::system::error_code const & ec, msgpack::object_handle oh){
            if (ec) {
                std::cerr << "[info][responser] error while handling request" << std::endl;
                return;
            }
            std::uint64_t id;
            std::string str;
            auto o = oh.get();
            o.via.array.ptr[1].convert(id);
            o = o.via.array.ptr[2];
            if (!assert_(o.type == msgpack::type::ARRAY, "invalid request, request should be an array")) return;
            if (!assert_(o.via.array.size > 1, "invalid request, array size should be greater then 1")) return;
            if (!assert_(o.via.array.ptr[0].type == msgpack::type::STR, "invalid request, request.op should be a string")) return;
            o.via.array.ptr[0].convert(str);
            o = o.via.array.ptr[1];
            auto i = m_map.find(str);
            std::cerr << "[info][responser] handle request " << str << std::endl;
            if (i == m_map.end()) {
                std::cerr << "[warning][responser] no request handler found: op="
                          << str
                          << ", obj="
                          << o
                          << std::endl;
                return;
            }
            oh.set(o);
            std::shared_ptr<msgpack::sbuffer> sp = std::make_shared<msgpack::sbuffer>();
            msgpack::packer<msgpack::sbuffer> pac(*sp);
            pac.pack_array(3);
            pac.pack_fix_uint8(2);
            pac.pack_fix_uint64(id);
            try {
                i->second(std::move(oh),*sp);
            } catch(...) {
                pac.pack_nil();
            }
            m_qw.async_write(
                *sp,
                [this,sp,id] (boost::system::error_code const & ec){
                    if (ec) {
                        std::cerr << "[info][responser] response error" << std::endl;
                    }
                    else {
                        std::cerr << "[info][responser] response done id#" << id << std::endl;
                    }
                });
        };

        void handle_post(boost::system::error_code const & ec, msgpack::object_handle oh){
            if (ec) {
                std::cerr << "[error][responser] error while handling post" << std::endl;
                return;
            }
            std::uint64_t id;
            std::string str;
            auto o = oh.get();
            o.via.array.ptr[1].convert(id);
            o = o.via.array.ptr[2];
            if (!assert_(o.type == msgpack::type::ARRAY, "[warning][responser] invalid post, post should be an array")) return;
            if (!assert_(o.via.array.size > 1, "[warning][responser] invalid post, array size should be greater then 1")) return;
            if (!assert_(o.via.array.ptr[0].type == msgpack::type::STR, "[warning][responser] invalid post, post.op should be a string")) return;
            o.via.array.ptr[0].convert(str);
            o = o.via.array.ptr[1];
            oh.set(o);
            auto i = m_post_map.find(str);
            std::cerr << "[info][responser] handle post " << str << std::endl;
            if (i == m_post_map.end()) {
                std::cerr << "[warning][responser] no post handler found: op="
                          << str
                          << ", obj="
                          << o
                          << std::endl;
                return;
            }
            try {
                i->second(std::move(oh));
            } catch(...) {
                std::cerr << "[error][responser] error occurs in post handler" << std::endl;
            }
        };

        template <typename SignatureT>
        void set_request_handler(std::string const & s, std::function<SignatureT> f) {
            m_map[s] = [ff = std::move(f)](msgpack::object_handle oh, msgpack::sbuffer & sbuf){
                using tuple_type = typename detail::signature_args_as_tuple<SignatureT>::type;
                tuple_type tup;
                try {
                    oh.get().convert(tup);
                    detail::apply_and_pack(sbuf,ff,tup);
                }
                catch(...) {
                    std::cerr << "[warning][responser] invalid request arguments" << std::endl;
                    msgpack::pack(sbuf,msgpack::type::nil_t{});
                }
            };
        }
        template <typename SignatureT>
        void set_post_handler(std::string const & s, std::function<SignatureT> f) {
            m_post_map[s] = [ff = std::move(f)](msgpack::object_handle oh){
                using tuple_type = typename detail::signature_args_as_tuple<SignatureT>::type;
                tuple_type tup;
                try {
                    oh.get().convert(tup);
                    api::apply(ff,tup);
                }
                catch(...) {
                    std::cerr << "[warning][responser] invalid post arguments" << std::endl;
                }
            };
        }
    private:
        std::map<std::string,pair_type::second_type> m_map;
        std::map<std::string,post_pair_type::second_type> m_post_map;
        msgpack_queued_writer & m_qw;
    };
} // namespace api
#endif // API_RESPONSER_HPP_INCLUDED
