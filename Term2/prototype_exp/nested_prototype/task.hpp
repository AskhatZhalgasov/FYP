#pragma once

#include <coroutine>
#include "threadpool.hpp"

class Task;

struct task_promise;

struct task_promise {
  CheckpointHelper checkpointer;
  Threadpool* m_threadpool;
  std::string name;
  
  explicit task_promise(std::string filename, Threadpool* pool) : name(filename), checkpointer(filename), m_threadpool(pool) {
    pool->set_status(filename, pool->Status::Running);
  //  std::cout << "constructor called, filename: " << filename << std::endl;
    if (checkpointer.is_file_exists()) {
      checkpointer.deserialize();
    }
  }

  Task get_return_object() noexcept;

  std::suspend_always initial_suspend() { return {}; }

  std::suspend_always final_suspend() noexcept { 
    
    this->m_threadpool->set_status(this->name, m_threadpool->Status::Finished);
    // in case its primary coroutine (i.e., called from main)
    if (this->m_continuation) 
      this->m_threadpool->set_status(m_continuation.promise().name, m_threadpool->Status::Running);
    return {}; 
  }
  
  void unhandled_exception() { abort(); }

  std::suspend_always yield_value(void* resume_point) {
    checkpointer.resume_point = resume_point; 
    checkpointer.serialize(); 
    return {};
  }

  void return_void() {}

  void set_continuation(std::coroutine_handle<task_promise> continuation) noexcept {
    m_continuation = continuation;
  }

private:
  std::coroutine_handle<task_promise> m_continuation; 
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
      bool await_ready() { 
        auto threadpool = this->m_handle.promise().m_threadpool;
        auto promise = this->m_handle.promise();
     //   std::cout << "[operator co_await] " << threadpool->get_status(promise.name) << std::endl;
        return threadpool->get_status(promise.name) == threadpool->Status::Finished; 
      }
      
      void await_suspend(std::coroutine_handle<promise_type> awaiting_coroutine) noexcept {
        
        auto awaiting_promise = awaiting_coroutine.promise();
        auto threadpool = awaiting_promise.m_threadpool;
        threadpool->set_status(awaiting_promise.name, threadpool->Status::Blocked);
        threadpool->enqueue_task(awaiting_promise.name, awaiting_coroutine);

        this->m_handle.promise().set_continuation(awaiting_coroutine);
      }

      void await_resume() noexcept {}

      std::coroutine_handle<promise_type> m_handle;
    };
    return awaiter{m_handle};
  }

  bool finished() {
    auto promise = m_handle.promise();
    return promise.m_threadpool->get_status(promise.name) == promise.m_threadpool->Status::Finished;
  }
private:
  std::coroutine_handle<promise_type> m_handle;
};

inline Task task_promise::get_return_object() noexcept {
  auto handle = std::coroutine_handle<task_promise>::from_promise(*this); 
//  std::cout << "[get_return_object] " << checkpointer.filename << std::endl;
  m_threadpool->enqueue_task(name, handle);  
  return Task{handle};
}

