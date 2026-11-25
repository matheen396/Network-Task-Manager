#include "netask/protocol.hpp"
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
}
int main() {
    try {
        const auto path=std::filesystem::temp_directory_path()/"netask-tests.db";
        std::filesystem::remove(path);
        auto store=std::make_shared<netask::TaskStore>(path); netask::TaskManager manager(store);
        auto created=manager.create("Plan | launch", "line one\nline two", "Ada");
        require(created.id==1,"id starts at one");
        require(manager.list().size()==1,"create persisted");
        manager.assign(1,"Grace"); manager.set_status(1,netask::Status::in_progress);
        manager.update(1,"Updated","details");
        require(manager.list()[0].assignee=="Grace","assignment");
        require(manager.list()[0].status==netask::Status::in_progress,"status");
        netask::TaskManager reloaded(store);
        require(reloaded.list().size()==1&&reloaded.list()[0].description=="details","reload from disk");
        auto reply=netask::handle_request(reloaded,"CREATE|Second|desc|Team");
        require(reply.rfind("OK|TASK|2|",0)==0,"wire protocol create");
        require(netask::handle_request(reloaded,"STATUS|2|bad").rfind("ERR|",0)==0,"protocol errors returned");
        reloaded.remove(1); require(reloaded.list().size()==1,"delete");
        bool missing=false; try{reloaded.remove(99);}catch(const std::out_of_range&){missing=true;}
        require(missing,"missing task rejected");
        std::filesystem::remove(path);
        std::cout << "All netask tests passed\n"; return 0;
    } catch(const std::exception& e) { std::cerr << "Test failure: " << e.what() << '\n'; return 1; }
}
