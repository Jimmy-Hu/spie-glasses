#ifndef API_IMAGE_SINK_HPP_INCLUDED
#define API_IMAGE_SINK_HPP_INCLUDED

// internal api
// external api
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <opencv2/opencv.hpp>
#include <msgpack.hpp>
// stdandard api
#include <atomic>
#include <thread>
#include <mutex>

namespace api {
    struct image_sink
    {
        image_sink()
            : m_thread()
            , m_is_running(false)
            {}
        void open() {
            m_is_running = true;
            m_thread = std::thread([this](){do_run();});
        }
        void show(std::string const & s, cv::Mat m) {
            std::lock_guard<std::mutex> guard(m_mutex);
            m_queue.push_back(std::make_pair(s,m));
        }
        void close() {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_is_running = false;
            lock.unlock();
            m_thread.join();
        }
        void do_run() {
            while (m_is_running) {
                while (true) {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    bool empty = m_queue.empty();
                    if (empty) break;
                    auto p = m_queue.front();
                    m_queue.pop_front();
                    lock.unlock();
                    cv::imshow(p.first,p.second);
                }
                cv::waitKey(27);
            }
        }
        bool is_opened() {
            return m_is_running;
        }
    private:
        std::thread m_thread;
        bool m_is_running;
        std::mutex m_mutex;
        std::deque<std::pair<std::string,cv::Mat>> m_queue;
    };

    // struct old_image_sink
    // {
    //     old_image_sink()
    //         : m_ios()
    //         , m_thread()
    //         , m_is_running(false)
    //         {}
    //     void open() {
    //         m_is_running = true;
    //         m_thread = std::thread([this](){do_run();});
    //     }
    //     void show(std::string const & s, cv::Mat m) {
    //         m_ios.post(
    //             [this,s,m]() {
    //                 if (!m.empty())
    //                     cv::imshow(s,m);
    //             });
    //     }
    //     void close() {
    //         m_ios.post([this](){m_is_running = false;});
    //         m_thread.join();
    //     }
    //     void do_run() {
    //         while (m_is_running) {
    //             m_ios.run();
    //             cv::waitKey(27);
    //         }
    //     }
    //     bool is_opened() {
    //         return m_is_running;
    //     }
    // private:
    //     boost::asio::io_service m_ios;
    //     std::thread m_thread;
    //     bool m_is_running;
    // };
} // namespace api

#endif // API_IMAGE_SINK_HPP_INCLUDED
