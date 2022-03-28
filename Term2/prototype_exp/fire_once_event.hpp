#pragma once 

#include <atomic>
#include <mutex>
#include <condition_variable>

struct fire_once_event {
  void set() {
    cv.notify_all();
  }

  void wait() {
    std::unique_lock<std::mutex> lk(mt);
    cv.wait(lk, []() { return false; } );
  }

private:
  std::condition_variable cv;
  std::mutex mt;
};
