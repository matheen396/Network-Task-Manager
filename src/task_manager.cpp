#include "netask/task_manager.hpp"
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace netask {
TaskManager::TaskManager(std::shared_ptr<TaskStore> store) : store_(std::move(store)) {
    if (!store_) throw std::invalid_argument("task store is required");
    tasks_ = store_->load();
    for (const auto& t : tasks_) next_id_ = std::max(next_id_, t.id + 1);
}
std::vector<Task> TaskManager::list() const { std::scoped_lock lock(mutex_); return tasks_; }
Task& TaskManager::find(std::uint64_t id) {
    auto it = std::find_if(tasks_.begin(), tasks_.end(), [id](const Task& t){ return t.id == id; });
    if (it == tasks_.end()) throw std::out_of_range("task not found");
    return *it;
}
void TaskManager::persist() { store_->save(tasks_); }
Task TaskManager::create(std::string title, std::string description, std::string assignee) {
    if (title.empty()) throw std::invalid_argument("title cannot be empty");
    std::scoped_lock lock(mutex_);
    Task t{next_id_++, std::move(title), std::move(description), std::move(assignee), Status::todo};
    tasks_.push_back(t);
    try { persist(); } catch (...) { tasks_.pop_back(); --next_id_; throw; }
    return t;
}
Task TaskManager::update(std::uint64_t id, std::string title, std::string description) {
    if (title.empty()) throw std::invalid_argument("title cannot be empty");
    std::scoped_lock lock(mutex_); auto& t = find(id); auto old = t;
    t.title = std::move(title); t.description = std::move(description);
    try { persist(); } catch (...) { t = std::move(old); throw; } return t;
}
Task TaskManager::assign(std::uint64_t id, std::string assignee) {
    std::scoped_lock lock(mutex_); auto& t = find(id); auto old = t.assignee; t.assignee = std::move(assignee);
    try { persist(); } catch (...) { t.assignee = std::move(old); throw; } return t;
}
Task TaskManager::set_status(std::uint64_t id, Status status) {
    std::scoped_lock lock(mutex_); auto& t = find(id); auto old = t.status; t.status = status;
    try { persist(); } catch (...) { t.status = old; throw; } return t;
}
void TaskManager::remove(std::uint64_t id) {
    std::scoped_lock lock(mutex_); auto it = std::find_if(tasks_.begin(), tasks_.end(), [id](const Task& t){return t.id == id;});
    if (it == tasks_.end()) throw std::out_of_range("task not found");
    auto pos = static_cast<std::size_t>(it - tasks_.begin()); auto old = *it; tasks_.erase(it);
    try { persist(); } catch (...) { tasks_.insert(tasks_.begin() + static_cast<std::ptrdiff_t>(pos), std::move(old)); throw; }
}
}
