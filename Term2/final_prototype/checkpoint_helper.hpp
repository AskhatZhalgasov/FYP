#pragma once

#include <bits/stdc++.h>

struct CheckpointHelper;

#define named_label(a, b) concat(a, b) 
#define concat(a, b) a ## b
#define unique_label named_label(label, __LINE__)

#define checkpoint_await_with_label(label_name, coro) \
  label_name:                                   \
  checkpointer->resume_point = &&label_name;    \
  co_await coro;                                

#define checkpoint_await(coro) \
  checkpoint_await_with_label(unique_label, coro)

#define checkpoint_with_label(label_name)     \
  label_name:                                 \
  checkpointer->resume_point = &&label_name;  \
  checkpointer->serialize();

#define checkpoint_save() \
  checkpoint_with_label(unique_label)

#define checkpoint_init() \
  auto checkpointer = co_await GetCheckpoint<Task::promise_type>{}; \
  if (checkpointer->resume_point != nullptr) \
    goto *checkpointer->resume_point; 


template<typename PromiseType> struct GetCheckpoint {
  CheckpointHelper* _checkpointer;
  bool await_ready() { return false; } // says yes call await_suspend
  bool await_suspend(std::coroutine_handle<PromiseType> handle) {
    _checkpointer = &handle.promise().checkpointer;
    return false;     // says no don't suspend coroutine after all
  }
  auto await_resume() { return _checkpointer; }
};

enum Status {
  Blocked = 0, 
  Running, 
  Finished
};

struct CheckpointHelper {
    std::string filepath;
    void* resume_point;    
    Status status;

    CheckpointHelper(std::string filename) : filepath(filename) {}

    /**
     * @brief 
     * 
     * @param checkpointer 
     * @param filename 
     */
    virtual void serialize() {

        //std::cout << "[serialization] filepath: " << filepath << ", status: " << status << ", resume_point: " << resume_point << std::endl;

        std::ostringstream stream;
        char null = 0;

        stream.write(reinterpret_cast<char*>(&resume_point), sizeof resume_point);
        stream.write(reinterpret_cast<char*>(&status), sizeof status);
        stream.write(&null, 1);

        std::ofstream storage;
        storage.open(filepath);
        storage << stream.str();
        storage.close();
    }

    /**
     * @brief 
     * 
     * @param checkpointer 
     * @param filename 
     */
    virtual void deserialize() {
        std::ifstream storage(filepath);
        if (storage.fail()) {
            // TODO: Format the runtime error message string
            throw std::runtime_error("File not exist exception");
        }

        storage.read(reinterpret_cast<char*>(&resume_point), sizeof resume_point);
        storage.read(reinterpret_cast<char*>(&status), sizeof status);

//        std::cout << "[deserialization] filepath: " << filepath << ", status: " << status << ", resume_point: " << resume_point << std::endl;

        storage.close();
    }

    bool is_file_exists() {
        return std::filesystem::exists(filepath);
    }
};
