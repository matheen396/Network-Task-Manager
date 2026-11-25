#include "netask/task.hpp"
#include <stdexcept>

namespace netask {
std::string to_string(Status status) {
    switch (status) {
    case Status::todo: return "todo";
    case Status::in_progress: return "in_progress";
    case Status::done: return "done";
    }
    throw std::invalid_argument("invalid task status");
}
Status status_from_string(const std::string& value) {
    if (value == "todo") return Status::todo;
    if (value == "in_progress") return Status::in_progress;
    if (value == "done") return Status::done;
    throw std::invalid_argument("status must be todo, in_progress, or done");
}
}
