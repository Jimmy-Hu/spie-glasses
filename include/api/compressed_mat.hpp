#ifndef API_COMPRESSED_MAT_HPP_INCLUDED
#define API_COMPRESSED_MAT_HPP_INCLUDED

// internal api
#include <api/functional.hpp>
// external api
#include <msgpack.hpp>
#include <opencv2/opencv.hpp>
#include <zlib.h>
// stdandard api
#include <iostream>
#include <utility>
#include <memory>
#include <vector>

namespace api {
    struct compressed_mat : cv::Mat
    {
        int mode;
        template <typename ...ArgTs>
        compressed_mat(ArgTs&&...args)
            : cv::Mat(std::forward<ArgTs>(args)...)
            , mode(0)
            {}
        compressed_mat(const compressed_mat & o)
            : cv::Mat(static_cast<const cv::Mat &>(o))
            , mode(o.mode)
            {}
        compressed_mat(const compressed_mat && o)
            : cv::Mat(static_cast<const cv::Mat &>(o))
            , mode(o.mode)
            {}
        compressed_mat(compressed_mat & o)
            : cv::Mat(static_cast<cv::Mat &>(o))
            , mode(o.mode)
            {}
        compressed_mat(compressed_mat && o)
            : cv::Mat(static_cast<cv::Mat &>(o))
            , mode(o.mode)
            {}
        compressed_mat & operator=(const compressed_mat & o)
            {
                *this = std::forward<decltype(o)&>(o);
                mode = o.mode;
            }
        compressed_mat & operator=(const compressed_mat && o)
            {
                *this = std::forward<decltype(o)&>(o);
                mode = o.mode;
            }
        compressed_mat & operator=(compressed_mat & o)
            {
                *this = std::forward<decltype(o)&>(o);
                mode = o.mode;
            }
        compressed_mat & operator=(compressed_mat && o)
            {
                *this = std::forward<decltype(o)&>(o);
                mode = o.mode;
            }
    };
} // namespace api

#include <opencv2/opencv.hpp>
#include <zlib.h>
#include <lz4.h>
#include <msgpack.hpp>
#include <vector>

namespace api {
    namespace detail {
    } // namespace detail
} // namespace api

namespace msgpack {
    MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
        namespace adaptor {
            template <>
            struct pack<api::compressed_mat> {
                template <typename Stream>
                msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, api::compressed_mat const& cm) const {
                    // packing implementation.
                    cv::Mat const & m = cm;
                    int mode = cm.mode;
                    bool is_continuous = m.isContinuous();
                    bool use_fallback = false;
                    o.pack_array(5);
                    o.pack(m.rows);
                    o.pack(m.cols);
                    o.pack(m.type());
                    std::size_t size = m.rows * m.cols * m.elemSize();
                    if (mode == 2)
                        do {
                            o.pack(2); // lz4 compress mode
                            cv::Mat mm = is_continuous ? m : m.clone();
                            if (!mm.isContinuous()) {
                                use_fallback = true;
                                break;
                            }
                            char * src = const_cast<char*>(reinterpret_cast<const char*>(mm.ptr()));
                            std::vector<char> buffer;
                            int limit = ::LZ4_compressBound(size);
                            buffer.resize(limit);
                            char * dest = const_cast<char*>(buffer.data());
                            int bin_size = ::LZ4_compress_default(src, dest, size, limit);
                            o.pack_bin(bin_size);
                            o.pack_bin_body(dest, bin_size);
                        } while(false);
                    else if (mode == 1)
                        do {
                            ::z_stream strm;
                            strm.zalloc = Z_NULL;
                            strm.zfree = Z_NULL;
                            strm.opaque = Z_NULL;
                            auto ret = ::deflateInit(&strm, Z_DEFAULT_COMPRESSION);
                            if (ret != Z_OK) {
                                std::cerr << "fallback [1]" << std::endl;
                                use_fallback = true;
                                ::deflateEnd(&strm);
                                break;
                            }
                            std::vector<char> buffer;
                            buffer.resize(::deflateBound(&strm,size));
                            strm.next_out = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(buffer.data()));
                            strm.avail_out = buffer.size();
                            if (is_continuous) {
                                strm.avail_in = size;
                                strm.next_in = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(m.ptr()));
                                auto ret = ::deflate(&strm, Z_FINISH);
                                if (ret == Z_STREAM_ERROR) {
                                    std::cerr << "fallback [2]" << std::endl;
                                    use_fallback = true;
                                    ::deflateEnd(&strm);
                                    break;
                                }
                            }
                            else {
                                for (int i = 0; i < m.rows; ++i) {
                                    strm.avail_in = m.cols * m.elemSize();
                                    strm.next_in = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(m.ptr(i)));
                                    auto ret = ::deflate(&strm, Z_NO_FLUSH);
                                    if (ret == Z_STREAM_ERROR) {
                                        std::cerr << "fallback [3]" << std::endl;
                                        use_fallback = true;
                                        ::deflateEnd(&strm);
                                        break;
                                    }
                                }
                                auto ret = ::deflate(&strm, Z_FINISH);
                                if (ret == Z_STREAM_ERROR) {
                                    std::cerr << "fallback [4]" << std::endl;
                                    use_fallback = true;
                                    ::deflateEnd(&strm);
                                    break;
                                }
                            }
                            if (strm.avail_in != 0) { /* all input will be used */
                                std::cerr << "fallback [5]" << std::endl;
                                use_fallback = true;
                                ::deflateEnd(&strm);
                                break;
                            }
                            ::deflateEnd(&strm);
                            std::size_t bin_size = buffer.size() - strm.avail_out;
                            o.pack(1); // gz compress mode
                            o.pack_bin(bin_size);
                            o.pack_bin_body(buffer.data(), bin_size);
                        } while (false);
                    else if (mode == 0 || use_fallback) {
                        o.pack(0); // non compress mode
                        o.pack_bin(size);
                        if (is_continuous) {
                            o.pack_bin_body(reinterpret_cast<const char*>(m.ptr()), size);
                        }
                        else {
                            for (int i = 0; i < m.rows; ++i) {
                                o.pack_bin_body(reinterpret_cast<const char*>(m.ptr(i)), m.cols * m.elemSize());
                            }
                        }
                        return o;
                    }
                }
            };
            template <>
            struct convert<api::compressed_mat> {
                msgpack::object const& operator()(msgpack::object const&o, api::compressed_mat&cm) const
                    {
                        cv::Mat & m = cm;
                        int & mode = cm.mode;
                        int rows,cols,type;
                        o.via.array.ptr[0].convert(rows);
                        o.via.array.ptr[1].convert(cols);
                        o.via.array.ptr[2].convert(type);
                        o.via.array.ptr[3].convert(mode);
                        auto bin = o.via.array.ptr[4].via.bin;
                        if (mode == 0) {
                            std::cerr << "normal mode " << std::endl;
                            m = cv::Mat(rows, cols, type, const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(bin.ptr))).clone();
                        }
                        else if (mode == 1) {// gz compress mode
                            std::cerr << "gz compress mode " << std::endl;
                            m = cv::Mat(rows, cols, type);
                            ::z_stream strm;
                            strm.zalloc = Z_NULL;
                            strm.zfree  = Z_NULL;
                            strm.opaque = Z_NULL;
                            ::inflateInit(&strm);
                            strm.next_in = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(bin.ptr));
                            strm.avail_in = bin.size;
                            strm.next_out  = reinterpret_cast<unsigned char*>(m.ptr());
                            strm.avail_out = m.rows * m.cols * m.elemSize();
                            ::inflate(&strm,Z_FINISH);
                        }
                        else if (mode == 2) {// gz compress mode
                            std::cerr << "lz4 compress mode " << std::endl;
                            m = cv::Mat(rows, cols, type);
                            std::size_t size = m.rows * m.cols * m.elemSize();
                            int originalSize = bin.size;
                            char * src = const_cast<char*>(reinterpret_cast<const char*>(bin.ptr));
                            char * dest = reinterpret_cast<char*>(m.ptr());
                            int limit = m.rows * m.cols * m.elemSize();
                            LZ4_decompress_safe(src, dest, originalSize, limit);
                        }
                        return o;
                    }
            };
        } // namespace adaptor
    } // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack

#endif // API_COMPRESSED_MAT_HPP_INCLUDED
