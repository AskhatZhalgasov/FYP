#pragma once

#include <coroutine>
#include "threadpool.hpp"

class [[nodiscard]] Task {
public:
    struct promise_type {
        CheckpointHelper checkpointer;

        explicit promise_type(std::string filename, Threadpool& pool) : checkpointer(filename) {
            if (checkpointer.is_file_exists()) {
                checkpointer.deserialize();
            }
        }

        Task get_return_object() {
            return Task { std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        std::suspend_always initial_suspend() { return {}; } 
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { abort(); }

        // address to serialize
        std::suspend_always yield_value(void* resume_point) {
            checkpointer.resume_point = resume_point; 
            checkpointer.serialize(); 
            return {};
        }
    };

    explicit Task() : _handle(std::coroutine_handle<promise_type>()) {}

    explicit Task(std::coroutine_handle<promise_type> handle) : _handle(handle) {}

    ~Task() { _handle.destroy(); }

    bool finished() {
        return _handle != NULL && _handle.done();
    }

    void resume() {
        return _handle.resume();
    }

    // TODO: Use function fetch_resumable_point() instead of struct GetResumePoint
    // Low-priority
    struct GetCheckpoint {
        CheckpointHelper* _checkpointer;
        bool await_ready() { return false; } // says yes call await_suspend
        bool await_suspend(std::coroutine_handle<Task::promise_type> handle) {
            _checkpointer = &handle.promise().checkpointer;
            return false;     // says no don't suspend coroutine after all
        }
        auto await_resume() { return _checkpointer; }
    };

    // auto fetch_resumable_point() {
    //     struct awaiter {
    //         CheckpointHelper* _checkpointer;
    //         bool await_ready() { return false; } // says yes call await_suspend
    //         bool await_suspend(std::coroutine_handle<Task::promise_type> handle) {
    //             _checkpointer = &handle.promise().checkpointer;
    //             return false;     // says no don't suspend coroutine after all
    //         }
    //         void* await_resume() { return return_point; }
    //     };
    //     return awaiter{this};
    // }

private:
    std::coroutine_handle<promise_type> _handle;
};
