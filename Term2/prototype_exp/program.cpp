#include <bits/stdc++.h>
#include <concepts>
#include <coroutine>
#include <exception>
#include <chrono>
#include <thread>
#include "checkpoint_helper.hpp"
#include "task.hpp"

#define named_label(a, b) concat(a, b)

#define concat(a, b) a ## b

#define unique_label named_label(label, __LINE__)

#define checkpoint_with_label(label_name) \
  co_yield &&label_name; \
  label_name:

#define checkpoint_suspend() \
  checkpoint_with_label(unique_label)

#define checkpoint_file "checkpoint.data"

using namespace std;

Task AlphaAsync(std::string filename) {
  // because we can't access promise_type within coroutine body
  // this is a hook to get return point address, then jump to it
  auto resume_point = co_await Task::GetResumePoint{};
  if (resume_point != nullptr)
    goto *resume_point;

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::cout << "PHASE 1\n";
  checkpoint_suspend();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::cout << "PHASE 2\n";
  checkpoint_suspend();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::cout << "PHASE 3 (final!)\n";
  checkpoint_suspend();
}


int main() {

  auto coro = AlphaAsync("checkpoint.data");

  while(!coro.finished()) {
    coro.resume();
    getchar(); // can simulate crash throug at this point
  }
  cout << "exited from coroutine\n";

  return 0;
}
