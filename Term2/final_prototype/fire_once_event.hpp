#pragma once 

#include <atomic>
#include <mutex>
#include <condition_variable>

struct fire_once_event {
  void set() {
    fired = true;
    cv.notify_all();
  }

  void wait() {
    std::unique_lock<std::mutex> lk(mt);
    cv.wait(lk, [&]() { return fired; } );
  }

private:
  std::condition_variable cv;
  std::mutex mt;
  bool fired = false;
};
