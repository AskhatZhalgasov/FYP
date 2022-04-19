#pragma once

#include <bits/stdc++.h>

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
