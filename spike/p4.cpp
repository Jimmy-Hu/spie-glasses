#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

bool is_ready(false);
std::mutex m;
std::condition_variable cv;

void
test()
{
    std::cout << "start sleep" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "end sleep" << std::endl;
    std::unique_lock<std::mutex> lk(m);
    is_ready = true;
    lk.unlock();
    cv.notify_one();
}

int
main()
{
    std::thread t(test);
    std::unique_lock<std::mutex> lk(m);
    while (!is_ready)
    {
        cv.wait(lk);
        if (!is_ready)
            std::cout << "Spurious wake up!\n";
    }
    t.join();
}
