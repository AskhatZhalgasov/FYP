#pragma once

#include <coroutine>
#include "threadpool.hpp"
#include "checkpoint_helper.hpp"

class Task;

struct task_promise;

struct task_promise {
  CheckpointHelper checkpointer;
  Threadpool* m_threadpool;
  std::string name;
  
  void set_status(Status status) {
    this->checkpointer.status = status;
  }

  Status get_status() {
    return this->checkpointer.status;
  }

  explicit task_promise(std::string filename, Threadpool* pool) : name(filename), checkpointer(filename), m_threadpool(pool) {
    set_status(Status::Running);
    //std::cout << "constructor called, filename: " << filename << std::endl;
    if (checkpointer.is_file_exists()) {
      //std::cout << "deserialization called\n";
      checkpointer.deserialize();
    }
  }

  Task get_return_object() noexcept;

  auto initial_suspend() { 
    struct awaiter {
      bool await_ready() {
        return false;
      }

      void await_suspend(std::coroutine_handle<task_promise> handle) {
        auto promise = handle.promise();
        //std::cout << "[initial suspend] suspended, thread: " << std::this_thread::get_id() << std::endl;
        promise.m_threadpool->enqueue_task(promise.name, handle);  
      }

      void await_resume() {
//        std::cout << "[initial suspend] resuming coroutine: " << std::this_thread::get_id() << std::endl;
      }
    };
    return awaiter{}; 
  }

  std::suspend_always final_suspend() noexcept { 
    
    set_status(Status::Finished);
    this->checkpointer.serialize();
    //std::cout << "obtained checkpointer\n";
    if (this->m_continuation) { 
      this->m_continuation.promise().set_status(Status::Running);
      this->m_threadpool->enqueue_task(m_continuation.promise().name, m_continuation);
    }
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
        return promise.get_status() == Status::Finished; 
      }
      
      void await_suspend(std::coroutine_handle<promise_type> awaiting_coroutine) noexcept {
        
        this->m_handle.promise().set_continuation(awaiting_coroutine);

        auto awaiting_promise = awaiting_coroutine.promise();
        auto threadpool = awaiting_promise.m_threadpool;
        awaiting_promise.set_status(Status::Blocked);
        awaiting_promise.checkpointer.serialize();
      }

      void await_resume() noexcept { 
      }

      std::coroutine_handle<promise_type> m_handle;
    };
    return awaiter{m_handle};
  }

  bool finished() {
    auto promise = m_handle.promise();
    //std::cout << "[Finished], status: " << promise.get_status() << ", address: " << m_handle.address() << std::endl;
    return promise.get_status() == Status::Finished;
  }

private:
  std::coroutine_handle<promise_type> m_handle;
};

inline Task task_promise::get_return_object() noexcept {
  auto handle = std::coroutine_handle<task_promise>::from_promise(*this); 
//  std::cout << "[get_return_object] " << checkpointer.filename << std::endl;
  return Task{handle};
}

