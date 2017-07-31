#include <iostream>
#include <vector>
#include <deque>
#include <functional>

int main()
{
    auto f = [a = 0](){};
    std::cout << f.a << std::endl;
}

struct io_service {
    std::deque<std::function<void()>> m_task_queue;
    void post(std::function<void()>);
    void run();
};

void io_service::post(std::function<void()> fn)
{
    m_task_queue.emplace_back(std::move(fn));
}

void io_service::run()
{
    while (!m_task_queue.empty()) {
        m_task_queue.back()();
        m_task_queue.pop_back();
    }
}

struct base_source {
    virtual get(int & x, std::function<void()>) = 0;
    virtual ~base_source() = 0;
};

struct base_sink {
    virtual put(int x, std::function<void()>) = 0;
    virtual ~base_sink() = 0;
};

// struct source : base_source {
//     io_service & m_ios;
//     std::function<void(base_sink &)> m_fn;
//     source(io_service & ios, std::function<void(base_sink &)> fn);
// };

// struct sink : base_sink {
//     io_service & m_ios;
//     std::function<void(base_sink &)> m_fn;
//     sink(io_service & ios, std::function<void(base_sink &)> fn);
// };


// source::source(io_service & ios, std::function<void(base_sink &)> fn)
//     : m_ios(ios), m_fn(std::move(fn))
// {}

// source::get(int & x, std::function<void()> on_get_done)
// {
// }
