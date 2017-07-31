#include <opencv2/opencv.hpp>
#include <iostream>
#include <fmt/format.h>
#include <msgpack.hpp>

namespace msgpack {
    MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
        namespace adaptor {
            template <>
            struct pack<cv::Mat> {
                template <typename Stream>
                msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, cv::Mat const& m) const {
                    // packing implementation.
                    bool is_continuous = m.isContinuous();
                    o.pack_array(4);
                    o.pack(m.rows);
                    o.pack(m.cols);
                    o.pack(m.type());
                    o.pack_bin(m.rows * m.cols * m.elemSize());
                    if (is_continuous) {
                        o.pack_bin_body(reinterpret_cast<const char*>(m.ptr()), m.rows * m.cols * m.elemSize());
                    } else {
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

// template<class Archive>
// void serialize(Archive &ar, cv::Mat& mat, const unsigned int)
// {
//     int cols, rows, type;
//     bool continuous;

//     if (Archive::is_saving::value) {
//         cols = mat.cols; rows = mat.rows; type = mat.type();
//         continuous = mat.isContinuous();
//     }

//     ar & cols & rows & type & continuous;

//     if (Archive::is_loading::value)
//         mat.create(rows, cols, type);

//     if (continuous) {
//         const unsigned int data_size = rows * cols * mat.elemSize();
//         ar & boost::serialization::make_array(mat.ptr(), data_size);
//     } else {
//         const unsigned int row_size = cols*mat.elemSize();
//         for (int i = 0; i < rows; i++) {
//             ar & boost::serialization::make_array(mat.ptr(i), row_size);
//         }
//     }
// }
int main(int argc, char** argv)
{
  int a;
  // cv::namedWindow("cv-camera", cv::WINDOW_AUTOSIZE);
  cv::VideoCapture cap;
  cap.open(0);
  if (!cap.isOpened()){
      fmt::print("default camera opened failed.\n");
    return -1;
  }
  fmt::print("default camera opened successfully.\n");
  double fps = cap.get(cv::CAP_PROP_POS_MSEC);
  if (fps <= 0) {
    fmt::print("fps got from video is zero, set fps to 30\n");
    fps = 30;
  }
  size_t msPerFrame = 1000 / fps;
  fmt::print("frame per seconds : {}\n", fps);
  fmt::print("millisconds per frame : {}\n", msPerFrame);
  cv::namedWindow("cv-camera");
  while (true) {
    cv::Mat frame;
    cv::Mat frame2;
    bool isFailed = !cap.read(frame);
    if (isFailed) {
      fmt::print("cannot read the frame from video file\n");
      break;
    }
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf,frame);
    msgpack::object_handle oh = msgpack::unpack(sbuf.data(), sbuf.size());
    cv::imshow("a", oh.get().as<cv::Mat>());
    cv::imshow("cv-camera", frame);
    if (cv::waitKey(msPerFrame) == 27) {
      fmt::print("a key pressed, exit this program.\n");
      break;
    }
  }
  return 0;
}
// int main() {
//     cv::Mat img2(24, 32, CV_8UC3, cv::Scalar(123,70,194));
//     msgpack::sbuffer sbuf;
//     msgpack::pack(sbuf,img2);
//     msgpack::object_handle oh = msgpack::unpack(sbuf.data(), sbuf.size());
//     std::cout << oh.get() << std::endl;
//     cv::imshow("a", oh.get().as<cv::Mat>());
//     cv::waitKey(0);
//     // oh.get().convert<cv::Mat>(frame2);
// }
