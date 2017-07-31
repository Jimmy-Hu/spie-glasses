#ifndef API_CV_MSGPACK_HPP_INCLUDED
#define API_CV_MSGPACK_HPP_INCLUDED
#include <msgpack.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

namespace msgpack {
    MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
        namespace adaptor {
            template <>
            struct pack<cv::Mat> {
                template <typename Stream>
                msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, cv::Mat const& m) const {
                    // packing implementation.
                    bool is_continuous = m.isContinuous();
                    std::size_t size = m.rows * m.cols * m.elemSize();
                    o.pack_array(4);
                    o.pack(m.rows);
                    o.pack(m.cols);
                    o.pack(m.type());
                    o.pack_bin(size);
                    if (is_continuous) {
                        std::cerr << "continuous Mat" << std::endl;
                        o.pack_bin_body(reinterpret_cast<const char*>(m.ptr()), size);
                    }
                    else {
                        std::cerr << "non-continuous Mat" << std::endl;
                        for (int i = 0; i < m.rows; ++i) {
                            o.pack_bin_body(reinterpret_cast<const char*>(m.ptr(i)), m.cols * m.elemSize());
                        }
                    }
                    return o;
                }
            };
            template <>
            struct convert<cv::Mat> {
                msgpack::object const& operator()(msgpack::object const&o, cv::Mat&m) const
                    {
                        int rows,cols,type;
                        o.via.array.ptr[0].convert(rows);
                        o.via.array.ptr[1].convert(cols);
                        o.via.array.ptr[2].convert(type);
                        auto bin = o.via.array.ptr[3].via.bin;
                        m = cv::Mat(rows,cols,type,const_cast<char*>(bin.ptr)).clone();
                        return o;
                    }
            };
        } // namespace adaptor
    } // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
namespace api {
} // namespace api

#endif // API_CV_MSGPACK_HPP_INCLUDED
