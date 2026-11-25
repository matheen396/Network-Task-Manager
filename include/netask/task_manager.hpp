#pragma once
#include "netask/task.hpp"
#include "netask/task_store.hpp"
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

namespace netask {
class TaskManager {
public:
    explicit TaskManager(std::shared_ptr<TaskStore> store);
    std::vector<Task> list() const;
    Task create(std::string title, std::string description, std::string assignee);
    Task update(std::uint64_t id, std::string title, std::string description);
    Task assign(std::uint64_t id, std::string assignee);
    Task set_status(std::uint64_t id, Status status);
    void remove(std::uint64_t id);
private:
    Task& find(std::uint64_t id);
    void persist();
    std::shared_ptr<TaskStore> store_;
    mutable std::mutex mutex_;
    std::vector<Task> tasks_;
    std::uint64_t next_id_{1};
};
}
