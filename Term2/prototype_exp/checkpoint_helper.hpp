#pragma once

class CheckpointHelper {
public:
    struct promise_type {
        void* resume_point;    
    };

    /**
     * @brief 
     * 
     * @param checkpointer 
     * @param filename 
     */
    static void serialize(promise_type& checkpointer, std::string filename) {
        std::ostringstream stream;
        char null = 0;

        stream.write(reinterpret_cast<char*>(&checkpointer.resume_point), sizeof checkpointer.resume_point);
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
    static void deserialize(promise_type& checkpointer, std::string filename) {
        std::ifstream storage(filename);
        if (storage.fail()) {
            // TODO: Format the runtime error message string
            throw std::runtime_error("File not exist exception");
        }

        storage.read(reinterpret_cast<char*>(&checkpointer.resume_point), sizeof checkpointer.resume_point);
        storage.close();
    }
};
