#include <iostream>
#include <thread>
// #include <boost/asio.hpp>
#include <fmt/format.h>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <boost/thread.hpp>
// #include <windows.h>
// #include <conio.h>
// #include <stdio.h>
#include <thread>


// std::mutex m;
// std::condition_variable cv;
// bool quit_ = false;
// int thread_main() {
//     std::string s;
//     // std::unique_lock<std::mutex> lk(m,std::defer_lock);
//     // fmt::print("start thread:", std::this_thread::get_id());
//     fmt::print("[thread] start...\n");
//     while (std::getline(std::cin,s)) {
//         if (s == "q") {
//             // lk.lock();
//             // fmt::print("[thread] 'q' pressed\n");
//             // fmt::print("[thread] 'q' accuire lock\n");
//             // ::quit_ = true;
//             fmt::print("[thread] unlock\n");
//             // lk.unlock();
//             // boost::unique_lock lk(m);
//             cv.notify_all();
//             fmt::print("[thread] notify\n");
//         }
//         // fmt::print("char: {}\n", s);
//         // std::this_thread::yield();
//         std::this_thread::sleep_for(std::chrono::milliseconds(50));
//     }
//     return 0;
// }

// struct conin_object
// {
//     conin_object();
//     void async_read_some();
//     void read_some();
// };
int main()
{
    // std::thread t(thread_main);
    std::vector<char> v(10);
    std::cin.readsome()
    return 0;
}
