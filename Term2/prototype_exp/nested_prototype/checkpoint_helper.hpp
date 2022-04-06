#pragma once

#include <bits/stdc++.h>

struct CheckpointHelper {
    std::string filename;
    void* resume_point;    

    CheckpointHelper(std::string filename) : filename(filename) {}

    /**
     * @brief 
     * 
     * @param checkpointer 
     * @param filename 
     */
    virtual void serialize() {
        std::ostringstream stream;
        char null = 0;

        stream.write(reinterpret_cast<char*>(&resume_point), sizeof resume_point);
        stream.write(&null, 1);

        std::ofstream storage;
        storage.open(filename);
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
        std::ifstream storage(filename);
        if (storage.fail()) {
            // TODO: Format the runtime error message string
            throw std::runtime_error("File not exist exception");
        }

        storage.read(reinterpret_cast<char*>(&resume_point), sizeof resume_point);
        storage.close();
    }

    bool is_file_exists() {
        return std::filesystem::exists(filename);
    }
};
