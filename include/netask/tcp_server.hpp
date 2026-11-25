#pragma once
#include "netask/task_manager.hpp"
#include <cstdint>
#include <memory>

namespace netask {
class TcpServer {
public:
    TcpServer(TaskManager& manager, std::uint16_t port);
    ~TcpServer();
    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;
    void run();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
