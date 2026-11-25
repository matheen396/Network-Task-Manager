#include "netask/task_store.hpp"
#include "netask/protocol.hpp"
#include <fstream>
#include <stdexcept>

namespace netask {
TaskStore::TaskStore(std::filesystem::path path) : path_(std::move(path)) {}
std::vector<Task> TaskStore::load() {
    std::scoped_lock lock(mutex_);
    std::vector<Task> tasks;
    std::ifstream in(path_);
    if (!in) {
        if (!std::filesystem::exists(path_)) return tasks;
        throw std::runtime_error("cannot open task data file: " + path_.string());
    }
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto f = split_fields(line);
        if (f.size() != 5) throw std::runtime_error("corrupt task data file");
        try {
            tasks.push_back({std::stoull(f[0]), decode_field(f[1]), decode_field(f[2]), decode_field(f[3]), status_from_string(f[4])});
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("invalid task data: ") + e.what());
        }
    }
    if (!in.eof()) throw std::runtime_error("error reading task data file");
    return tasks;
}
void TaskStore::save(const std::vector<Task>& tasks) {
    std::scoped_lock lock(mutex_);
    if (!path_.parent_path().empty()) std::filesystem::create_directories(path_.parent_path());
    auto temp = path_; temp += ".tmp";
    {
        std::ofstream out(temp, std::ios::trunc);
        if (!out) throw std::runtime_error("cannot write task data file: " + temp.string());
        for (const auto& t : tasks)
            out << t.id << '|' << encode_field(t.title) << '|' << encode_field(t.description) << '|'
                << encode_field(t.assignee) << '|' << to_string(t.status) << '\n';
        out.flush();
        if (!out) throw std::runtime_error("failed writing task data file");
    }
    std::error_code ec;
    std::filesystem::rename(temp, path_, ec);
    if (ec) {
        std::filesystem::remove(path_, ec); ec.clear();
        std::filesystem::rename(temp, path_, ec);
        if (ec) throw std::runtime_error("cannot replace task data file: " + ec.message());
    }
}
}
