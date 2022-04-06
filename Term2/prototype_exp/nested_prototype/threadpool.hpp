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

    /*
    auto schedule() {
        struct awaiter {
            Threadpool* m_threadpool;

            constexpr bool await_ready() const noexcept { return false; }
            constexpr void await_resume() const noexcept { }
            void await_suspend(std::coroutine_handle<promise_type> coro) const noexcept {
                m_threadpool->enqueue_task(coro);
            }
        };
        return awaiter{this};
    }
    */

    std::list<std::thread> m_threads;

    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::queue<std::pair<std::string, std::coroutine_handle<>>> m_coros;

    enum Status {
      Blocked = 0, 
      Running, 
      Finished
    };
    std::mutex stat_mt;
    std::unordered_map<std::string, Status> status; 

    void set_status(std::string name, Status new_status) {
      std::unique_lock<std::mutex> lk(stat_mt);
      status[name] = new_status;
    }

    Status get_status(std::string name) {
      std::unique_lock<std::mutex> lk(stat_mt);
      auto answer = status.at(name);
      return answer;
    }

    bool m_stop_thread = false;

    void thread_loop() {
        while (!m_stop_thread) {
            std::unique_lock<std::mutex> lock(m_mutex);

            while (!m_stop_thread && m_coros.size() == 0) {
                m_cond.wait_for(lock, std::chrono::microseconds(100));
            }

            if (m_stop_thread) {
                break;
            }

            auto [name, coro] = m_coros.front();
            m_coros.pop();
            lock.unlock();
            auto state = get_status(name);
            //std::cout << "[task pool], thread_id: " << std::this_thread::get_id() << ", size: " << m_coros.size() << ", name: " << name << ", state: " << state << std::endl;
            if (state == Status::Running) {
              coro.resume();
            } else if (state == Status::Blocked) {
              this->enqueue_task(name, coro);
            }   
        }
    }

    void enqueue_task(std::string name, std::coroutine_handle<> coro) noexcept {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_coros.emplace(make_pair(name, coro));
        m_cond.notify_one();
//        std::cout << "[enqueue task], name: " << name << ", " << status[name] << std::endl;
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
