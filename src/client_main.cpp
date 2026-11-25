#include "netask/protocol.hpp"
#include "netask/tcp_client.hpp"
#include <iostream>
#include <stdexcept>

namespace {
void usage() {
    std::cerr << "Usage: netask [host] [port] <command> [arguments]\n"
              << "  list | create <title> <description> <assignee> | update <id> <title> <description>\n"
              << "  assign <id> <assignee> | status <id> <todo|in_progress|done> | delete <id> | ping\n";
}
}
int main(int argc,char** argv) {
    if(argc<4){usage();return 2;}
    try {
        std::string host=argv[1]; auto port=static_cast<std::uint16_t>(std::stoul(argv[2])); std::string cmd=argv[3], req;
        auto arg=[&](int i)->std::string { if(i>=argc) throw std::invalid_argument("missing argument"); return argv[i]; };
        if(cmd=="list"||cmd=="ping") req=cmd=="list"?"LIST":"PING";
        else if(cmd=="create"&&argc==7) req="CREATE|"+netask::encode_field(arg(4))+"|"+netask::encode_field(arg(5))+"|"+netask::encode_field(arg(6));
        else if(cmd=="update"&&argc==7) req="UPDATE|"+arg(4)+"|"+netask::encode_field(arg(5))+"|"+netask::encode_field(arg(6));
        else if(cmd=="assign"&&argc==6) req="ASSIGN|"+arg(4)+"|"+netask::encode_field(arg(5));
        else if(cmd=="status"&&argc==6) req="STATUS|"+arg(4)+"|"+arg(5);
        else if(cmd=="delete"&&argc==5) req="DELETE|"+arg(4);
        else {usage();return 2;}
        std::cout << netask::tcp_request(host,port,req) << '\n'; return 0;
    } catch(const std::exception& e) { std::cerr << "client error: " << e.what() << '\n'; return 1; }
}
