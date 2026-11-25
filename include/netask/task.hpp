#pragma once
#include <cstdint>
#include <string>

namespace netask {
enum class Status { todo, in_progress, done };
std::string to_string(Status status);
Status status_from_string(const std::string& value);

struct Task {
    std::uint64_t id{};
    std::string title;
    std::string description;
    std::string assignee;
    Status status{Status::todo};
};
}
