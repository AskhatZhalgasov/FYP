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
  checkpoint->resume_point = &&label; \
  CheckpointHelper::serialize(checkpoint, "checkpoint.data");\
  co_await pool.schedule(); \
  label_name:

#define suspend_with(checkpoint, pool) \
  label_suspend_with(checkpoint, pool, unique_label)

Task AlphaAsync(std::string filename, Threadpool& pool) {
  // because we can't access promise_type within coroutine body
  // this is a hook to get return point address, then jump to it
  auto resume_point = co_await Task::GetResumePoint{};
  if (resume_point != nullptr)
    goto *resume_point;

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::cout << "PHASE 1\n";
  //checkpoint_suspend();
  suspend_with(resume_point, pool);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::cout << "PHASE 2\n";
  // checkpoint_suspend();
  suspend_with(resume_point, pool);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::cout << "PHASE 3 (final!)\n";
  // checkpoint_suspend();
  suspend_with(resume_point, pool);
}

Task run_async_print(std::string filename, Threadpool& pool) {
  std::cout << "This is a hello from thread: " << std::this_thread::get_id() << " before co_await is called with filename " << filename << "\n";
  co_await pool.schedule();
  std::cout << "This is a hello from thread: " << std::this_thread::get_id() << " after co_await is called with filename " << filename << "\n";
}

int main() {
  Threadpool pool{8};

  // auto task = run_async_print("async_print.data", pool);
  // std::this_thread::sleep_for(std::chrono::microseconds(1000));

  // return 0;
  auto coro = AlphaAsync("checkpoint.data", pool);

  while(!coro.finished()) {
    coro.resume();
    getchar(); // can simulate crash throug at this point
  }
  std::cout << "exited from coroutine\n";

  return 0;
}
