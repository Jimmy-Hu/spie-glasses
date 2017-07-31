#include <api/cv_msgpack.hpp>
#include <vector>

int main(int argc, char ** argv)
{
    if (argc < 2) {
        std::cerr << "no input image." << std::endl;
        return 0;
    }
    try {
        cv::Mat m = cv::imread(argv[1], CV_LOAD_IMAGE_UNCHANGED);
        // cv::Mat m(120,100,CV_8UC3, cv::Scalar(0xee,0xdd,0xcc));
        msgpack::sbuffer sbuf;
        msgpack::packer<msgpack::sbuffer> pac(sbuf);
        pac.pack(m);
        std::cout.write(sbuf.data(), sbuf.size());
        std::cout.flush();
    }
    catch (std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
