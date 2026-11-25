#include "netask/tcp_server.hpp"
#include "netask/protocol.hpp"
#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using socket_t = SOCKET;
static constexpr socket_t invalid_socket = INVALID_SOCKET;
static void close_socket(socket_t s){ closesocket(s); }
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using socket_t = int;
static constexpr socket_t invalid_socket = -1;
static void close_socket(socket_t s){ close(s); }
#endif

namespace netask {
struct TcpServer::Impl {
    TaskManager& manager; std::uint16_t port; socket_t listener{invalid_socket};
    std::atomic<bool> stopping{false}; std::mutex threads_mutex; std::vector<std::jthread> clients;
    Impl(TaskManager& m,std::uint16_t p):manager(m),port(p){}
};
TcpServer::TcpServer(TaskManager& m,std::uint16_t p):impl_(std::make_unique<Impl>(m,p)){}
TcpServer::~TcpServer(){
    if (!impl_) return;
    impl_->stopping=true;
    if (impl_->listener!=invalid_socket) { close_socket(impl_->listener); impl_->listener=invalid_socket; }
#ifdef _WIN32
    WSACleanup();
#endif
}
void TcpServer::run() {
#ifdef _WIN32
    WSADATA data{}; if(WSAStartup(MAKEWORD(2,2),&data)!=0) throw std::runtime_error("Winsock startup failed");
#endif
    auto& x=*impl_; x.listener=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(x.listener==invalid_socket) throw std::runtime_error("socket creation failed");
    int reuse=1; setsockopt(x.listener,SOL_SOCKET,SO_REUSEADDR,reinterpret_cast<const char*>(&reuse),sizeof(reuse));
    sockaddr_in addr{}; addr.sin_family=AF_INET; addr.sin_addr.s_addr=htonl(INADDR_ANY); addr.sin_port=htons(x.port);
    if(bind(x.listener,reinterpret_cast<sockaddr*>(&addr),sizeof(addr))!=0) throw std::runtime_error("bind failed; port may be in use or unavailable");
    if(listen(x.listener,SOMAXCONN)!=0) throw std::runtime_error("listen failed");
    std::cout << "netask server listening on 0.0.0.0:" << x.port << '\n';
    while(!x.stopping) {
        sockaddr_in peer{};
#ifdef _WIN32
        int len=sizeof(peer);
#else
        socklen_t len=sizeof(peer);
#endif
        socket_t client=accept(x.listener,reinterpret_cast<sockaddr*>(&peer),&len);
        if(client==invalid_socket) { if(x.stopping) break; continue; }
        std::scoped_lock lock(x.threads_mutex);
        x.clients.emplace_back([client,&x](std::stop_token){
            std::string pending; char buf[2048];
            while(true) {
                int n=recv(client,buf,static_cast<int>(sizeof(buf)),0); if(n<=0) break;
                pending.append(buf,static_cast<std::size_t>(n));
                if(pending.size()>65536) break;
                std::size_t pos;
                while((pos=pending.find('\n'))!=std::string::npos) {
                    std::string line=pending.substr(0,pos); pending.erase(0,pos+1);
                    if(!line.empty()&&line.back()=='\r') line.pop_back();
                    auto response=handle_request(x.manager,line)+"\n.\n"; std::size_t sent=0;
                    while(sent<response.size()) { int k=send(client,response.data()+sent,static_cast<int>(response.size()-sent),0); if(k<=0){sent=response.size(); break;} sent+=static_cast<std::size_t>(k); }
                }
            }
            close_socket(client);
        });
    }
}
}
