#include <bits/stdc++.h>
#include <chrono>
#include <thread>
#include "checkpoint_helper.hpp"
#include "task.hpp"
#include "threadpool.hpp"
#include "sync_wait.hpp"

#define named_label(a, b) concat(a, b)

#define concat(a, b) a ## b

#define unique_label named_label(label, __LINE__)

#define checkpoint_with_label(label_name) \
  co_yield &&label_name; \
  label_name:

#define checkpoint_suspend() \
  checkpoint_with_label(unique_label)

#define label_suspend_with(checkpoint, pool, label_name) \
  checkpoint->resume_point = &&label_name; \
  checkpoint->serialize(); \
  co_await pool.schedule(); \
  label_name:

#define suspend_with(checkpoint, pool) \
  label_suspend_with(checkpoint, pool, unique_label)

template<typename PromiseType>
struct GetCheckpoint {
  CheckpointHelper* _checkpointer;
  bool await_ready() { return false; } // says yes call await_suspend
  bool await_suspend(std::coroutine_handle<PromiseType> handle) {
    _checkpointer = &handle.promise().checkpointer;
    return false;     // says no don't suspend coroutine after all
  }
  auto await_resume() { return _checkpointer; }
};

Task AlphaAsync(std::string filename, Threadpool& pool) {
  // because we can't access promise_type within coroutine body
  // this is a hook to get return point address, then jump to it

  auto checkpointer = co_await GetCheckpoint<Task::promise_type>{};
  if (checkpointer->resume_point != nullptr)
    goto *checkpointer->resume_point;

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::cout << "PHASE 1 with thread id: " << std::this_thread::get_id() << "\n";
  suspend_with(checkpointer, pool);
  getchar();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::cout << "PHASE 2 with thread id: " << std::this_thread::get_id() << "\n";
  suspend_with(checkpointer, pool);
  getchar();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::cout << "PHASE 3 with thread id: " << std::this_thread::get_id() << " [FINAL!]\n";
  suspend_with(checkpointer, pool);
  getchar();
}

int main() {
  Threadpool pool{4};
  auto task = AlphaAsync("checkpoint.data", pool);

  sync_wait(task); 

  return 0;
}
