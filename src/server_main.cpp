#include "netask/task_manager.hpp"
#include "netask/tcp_server.hpp"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

int main(int argc,char** argv) {
    try {
        std::uint16_t port=5050; std::string file="data/tasks.db";
        if(argc>1) port=static_cast<std::uint16_t>(std::stoul(argv[1]));
        if(const char* env=std::getenv("NETASK_DATA")) file=env;
        auto store=std::make_shared<netask::TaskStore>(file); netask::TaskManager manager(store);
        netask::TcpServer server(manager,port); server.run();
    } catch(const std::exception& e) { std::cerr << "server error: " << e.what() << '\n'; return 1; }
}
