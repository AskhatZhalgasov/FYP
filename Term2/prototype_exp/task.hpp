#pragma once

#include <coroutine>
#include "threadpool.hpp"

class Task;

struct task_promise {
  CheckpointHelper checkpointer;

  explicit task_promise(std::string filename, Threadpool& pool) : checkpointer(filename) {
    if (checkpointer.is_file_exists()) {
      checkpointer.deserialize();
    }
  }

  struct final_awaitable {
    bool await_ready() const noexcept { 
      return false;
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<task_promise> handle) noexcept {
      return handle.promise().m_continuation;
    } 

    void await_resume() noexcept {}
  };

  Task get_return_object() noexcept;
  std::suspend_always initial_suspend() { return {}; }
  final_awaitable final_suspend() noexcept { return {}; }
  void unhandled_exception() { abort(); }

  std::suspend_always yield_value(void* resume_point) {
    checkpointer.resume_point = resume_point; 
    checkpointer.serialize(); 
    return {};
  }

  void set_continuation(std::coroutine_handle<> continuation) noexcept {
    m_continuation = continuation;
  }

private:
  std::coroutine_handle<> m_continuation; 
};

class [[nodiscard]] Task {
public:
    
  using promise_type = task_promise;

  explicit Task() : m_handle(std::coroutine_handle<promise_type>()) {}

  explicit Task(std::coroutine_handle<promise_type> handle) : m_handle(handle) {}

  ~Task() { 
    if (m_handle) 
      m_handle.destroy();
  }

  auto operator co_await() {
    struct awaiter{
      bool await_ready() { return !this->m_handle or this->m_handle.done(); }
      
      std::coroutine_handle<> await_suspend(std::coroutine_handle<> awaiting_coroutine) noexcept {
        this->m_handle.promise().set_continuation(awaiting_coroutine);
        return this->m_handle;
      }

      void await_resume() noexcept {}

      std::coroutine_handle<promise_type> m_handle;
    };
    return awaiter{m_handle};
  }
private:
  std::coroutine_handle<promise_type> m_handle;
};

inline Task task_promise::get_return_object() noexcept {
  return Task{std::coroutine_handle<task_promise>::from_promise(*this)};
}

