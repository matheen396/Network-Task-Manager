#pragma once
#include <cstdint>
#include <string>
namespace netask { std::string tcp_request(const std::string& host, std::uint16_t port, const std::string& request); }
