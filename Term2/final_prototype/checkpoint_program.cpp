#include <bits/stdc++.h>
#include <chrono>
#include <thread>
#include "checkpoint_helper.hpp"
#include "task.hpp"
#include "threadpool.hpp"

Task ThetaAsync(Threadpool* pool) {
  checkpoint_init();

  std::cout << "[ThetaAsync] PHASE 1 with thread id: " << std::this_thread::get_id() << "\n";

  checkpoint_save(); 
  getchar(); // simulate crash

  std::cout << "[ThetaAsync] after checkpoint with thread id: " << std::this_thread::get_id() << std::endl;
}

Task BetaAsync(Threadpool* pool) {
  checkpoint_init();

  std::cout << "[BetaAsync] PHASE 1 with thread id: " << std::this_thread::get_id() << "\n";

  checkpoint_save(); 
  getchar(); // simulate crash

  std::cout << "[BetaAsync] after checkpoint with thread id: " << std::this_thread::get_id() << std::endl;
}

Task AlphaAsync(Threadpool* pool) {
  // because we can't access promise_type within coroutine body
  // this is a hook to get return point address, then jump to it
  checkpoint_init();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::cout << "[AlphaAsync] PHASE 1 with thread id: " << std::this_thread::get_id() << "\n";

  // Task (awaitable) -> awaiter  
  checkpoint_await(BetaAsync(pool));

  std::cout << "[AlphaAsync] PHASE 3 with thread id: " << std::this_thread::get_id() << "\n";
  co_return;
}

int main() {
  Threadpool pool{1};

  auto task = AlphaAsync(&pool);
  task.wait_completion();

  std::puts("finished\n");

  return 0;
}
