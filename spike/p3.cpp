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


std::mutex m;
std::condition_variable cv;
bool quit_ = false;
int thread_main() {
    std::string s;
    // std::unique_lock<std::mutex> lk(m,std::defer_lock);
    // fmt::print("start thread:", std::this_thread::get_id());
    fmt::print("[thread] start...\n");
    while (std::getline(std::cin,s)) {
        if (s == "q") {
            // lk.lock();
            // fmt::print("[thread] 'q' pressed\n");
            // fmt::print("[thread] 'q' accuire lock\n");
            // ::quit_ = true;
            fmt::print("[thread] unlock\n");
            // lk.unlock();
            // boost::unique_lock lk(m);
            cv.notify_all();
            fmt::print("[thread] notify\n");
        }
        // fmt::print("char: {}\n", s);
        // std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return 0;
}

int main()
{
    std::thread t(thread_main);
    // fmt::print("[main] start...\n");
    std::unique_lock<std::mutex> lk(m);
    // while (!quit_)
    // {
        fmt::print("[main] start wait...\n");
    // cv.wait(lk);
    // cv.wait(lk, [](){return true;});
        fmt::print("[main] end wait...\n");
        // if (!quit_)
    //         // std::cout << "Spurious wake up!\n";
    // }
    // fmt::print("[main] condition satisfied\n");
    lk.unlock();
    fmt::print("[main] notified, do quit...\n");
    t.join();
    // auto h = ::GetStdHandle(STD_INPUT_HANDLE);
    // while (true) {
    //     auto b = std::cin.rdbuf()->in_avail();
    //     fmt::print("{}\r",b);
    //     if (b)
    //         fmt::print("-----------\n");
    // }
    return 0;
}
