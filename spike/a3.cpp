#ifndef API_SAFE_QUEUE
#define API_SAFE_QUEUE

#include <queue>
#include <mutex>
#include <condition_variable>

// A threadsafe-queue.
template <class QueueT, class MutexT, class ConditionVariableT>
class safe_queue
{
public:
    using reference = typename QueueT::reference;
    using const_reference = typename QueueT::const_reference;
    safe_queue(void)
        : q()
        , m()
        , c()
    {}

    ~safe_queue(void)
    {}

    bool empty();
    void front();
    void back();
    // Add an element to the queue.
    void enqueue(T t)
    {
        std::lock_guard<std::mutex> lock(m);
        q.push(t);
        c.notify_one();
    }

    // Get the "front"-element.
    // If the queue is empty, wait till a element is avaiable.
    T dequeue(void)
    {
        std::unique_lock<std::mutex> lock(m);
        while(q.empty())
        {
            // release lock as long as the wait and reaquire it afterwards.
            c.wait(lock);
        }
        T val = q.front();
        q.pop();
        return val;
    }

private:
    std::queue<T> q;
    mutable std::mutex m;
    std::condition_variable c;
};

#endif // API_SAFE_QUEUE
