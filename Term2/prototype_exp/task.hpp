#pragma once

class [[nodiscard]] Task : CheckpointHelper {
public:
    struct promise_type {
        CheckpointHelper::promise_type checkpointer;
        std::string checkpoint_filename;

        explicit promise_type(std::string& filename) : checkpoint_filename(filename) {
            if (std::filesystem::exists(checkpoint_filename)) {
                CheckpointHelper::deserialize(checkpointer, checkpoint_filename);
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
            CheckpointHelper::serialize(checkpointer, checkpoint_filename); 
            return {};
        }
    };

    explicit Task(std::coroutine_handle<promise_type> handle)
        : _handle(handle) {}

    bool finished() {
        return _handle != NULL && _handle.done();
    }

    void resume() {
        return _handle.resume();
    }

    /**
     * @brief TODO: Use auto get_resume_point() instead of using struct GetResumePoint
     * 
     */
    struct GetResumePoint {
        void* return_point;
        bool await_ready() { return false; } // says yes call await_suspend
        bool await_suspend(std::coroutine_handle<Task::promise_type> handle) {
            return_point = handle.promise().checkpointer.resume_point;
            return false;     // says no don't suspend coroutine after all
        }
        void* await_resume() { return return_point; }
    };

private:
    std::coroutine_handle<promise_type> _handle;
};
