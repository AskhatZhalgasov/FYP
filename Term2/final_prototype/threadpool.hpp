#pragma once

#include <condition_variable> 
#include <coroutine> 
#include <cstdint>
#include <list>
#include <mutex>
#include <queue>
#include <thread>

class Threadpool {
public:
    explicit Threadpool(const std::size_t threadCount) {
      for (std::size_t i = 0; i < threadCount; ++i) {
          std::thread worker_thread([this]() {
              this->thread_loop();
          });
          m_threads.push_back(std::move(worker_thread));
      }
    }

    ~Threadpool() {
        shutdown();
    }

    std::list<std::thread> m_threads;

    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::queue<std::pair<std::string, std::coroutine_handle<>>> m_coros;

    bool m_stop_thread = false;

    void thread_loop() {
        while (!m_stop_thread) {
            std::unique_lock<std::mutex> lock(m_mutex);
    //        std::cout << "[spinning thread], id: " << std::this_thread::get_id() << ", sz(m_coros): " << m_coros.size() << ", m_stop_thread: " << m_stop_thread << std::endl;

            while (!m_stop_thread && m_coros.size() == 0) {
              //std::cout << "[spinning thread], waiting, threadId: " << std::this_thread::get_id() << std::endl;
              m_cond.wait_for(lock, std::chrono::microseconds(100));
            }

            if (m_stop_thread) {
                break;
            }

            auto [name, coro] = m_coros.front();
            m_coros.pop();
            lock.unlock();
     //       std::cout << "[task pool], thread_id: " << std::this_thread::get_id() << ", size: " << m_coros.size() << ", name: " << name << ", coro(address): " << coro.address() << std::endl;
            coro.resume();
        }
    }

    void enqueue_task(std::string name, std::coroutine_handle<> coro) noexcept {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_coros.emplace(make_pair(name, coro));
//        std::cout << "[enqueue task], name: " << name << ", thread: " << std::this_thread::get_id() << ", coro(address): " << coro.address() << std::endl;
        lock.unlock();
        m_cond.notify_one();
        /*{
          std::cout << "[returning]\n";
          auto [name, coro_new] = m_coros.front();
          m_coros.pop();
//          coro_new.resume(); // resume 0x55555557beb0
        }*/
    }

    void shutdown() {
        m_stop_thread = true;
        while (m_threads.size() > 0) {
            std::thread& thread = m_threads.back();
            if (thread.joinable()) {
                thread.join();
            }
            m_threads.pop_back();
        }
    }
};
