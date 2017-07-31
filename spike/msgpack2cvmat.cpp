#include <api/cv_msgpack.hpp>
#include <msgpack.hpp>
#include <cstdio>

int main() {
    msgpack::unpacker unp;
    msgpack::object_handle oh;
    bool show = false;
    while (true) {
        unp.reserve_buffer(4096);
        std::size_t n = std::fread(unp.buffer(), 1, 4096, stdin);
        unp.buffer_consumed(n);
        while (unp.next(oh)) {
            std::cerr << "show image" << std::endl;
            cv::Mat m;
            try {
                oh.get().convert(m);
                cv::imshow("image", m);
                show = true;
                cv::waitKey(30);
            }
            catch (std::exception & e) {
                std::cerr << "error:" << e.what() << std::endl;
            }
        }
        if (std::feof(stdin)) {
            break;
        }
    }
    if (show)
        cv::waitKey(0);
    return 0;
}
