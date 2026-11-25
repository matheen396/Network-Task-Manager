#pragma once
#include "netask/task_manager.hpp"
#include <string>
#include <vector>

namespace netask {
std::string handle_request(TaskManager& manager, const std::string& request);
std::string encode_field(const std::string& value);
std::string decode_field(const std::string& value);
std::vector<std::string> split_fields(const std::string& line);
}
