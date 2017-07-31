#include <opencv2/opencv.hpp>
#include <thread>
#include <string>
int main() {
    std::thread t([](){/* cv::namedWindow("server"); */ while (cv::waitKey(30) != 27);});
    std::string s;
    std::getline(std::cin, s);

}
