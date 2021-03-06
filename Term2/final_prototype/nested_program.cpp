#include <bits/stdc++.h>
#include <chrono>
#include <thread>
#include "checkpoint_helper.hpp"
#include "task.hpp"
#include "threadpool.hpp"

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

#define suspend_with(checkpoint, pool) \ label_suspend_with(checkpoint, pool, unique_label)
template<typename PromiseType> struct GetCheckpoint {
  CheckpointHelper* _checkpointer;
  bool await_ready() { return false; } // says yes call await_suspend
  bool await_suspend(std::coroutine_handle<PromiseType> handle) {
    _checkpointer = &handle.promise().checkpointer;
    return false;     // says no don't suspend coroutine after all
  }
  auto await_resume() { return _checkpointer; }
};

Task ThetaAsync(std::string filename, Threadpool* pool) {
  std::cout << "[ThetaAsync] PHASE 1 with thread id: " << std::this_thread::get_id() << "\n";

  co_return;
}

Task BetaAsync(std::string filename, Threadpool* pool) {
  std::cout << "[BetaAsync] PHASE 1 with thread id: " << std::this_thread::get_id() << "\n";
  
  co_await ThetaAsync("theta_async.data", pool);

  std::cout << "[BetaAsync] PHASE 2 with thread id: " << std::this_thread::get_id() << "\n";
  co_return;
}

Task AlphaAsync(std::string filename, Threadpool* pool) {
  // because we can't access promise_type within coroutine body
  // this is a hook to get return point address, then jump to it
  auto checkpointer = co_await GetCheckpoint<Task::promise_type>{};
  std::cout << "obtained checkpointer\n";
  if (checkpointer->resume_point != nullptr)
    goto *checkpointer->resume_point;

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::cout << "[AlphaAsync] PHASE 1 with thread id: " << std::this_thread::get_id() << "\n";

  // Task (awaitable) -> awaiter  
  co_await BetaAsync("beta_async.data", pool);

  std::cout << "[AlphaAsync] PHASE 2 with thread id: " << std::this_thread::get_id() << "\n";
  co_return;
}

int main() {
  Threadpool pool{4};
  auto primary_task = AlphaAsync("alpha_async.data", &pool);

  while(!primary_task.finished());

  std::cout << "Finished!\n";

  return 0;
}
