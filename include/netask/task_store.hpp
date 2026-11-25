#pragma once
#include "netask/task.hpp"
#include <filesystem>
#include <mutex>
#include <vector>

namespace netask {
class TaskStore {
public:
    explicit TaskStore(std::filesystem::path path);
    std::vector<Task> load();
    void save(const std::vector<Task>& tasks);
private:
    std::filesystem::path path_;
    std::mutex mutex_;
};
}
