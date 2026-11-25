#include "netask/tcp_client.hpp"
#include <stdexcept>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using socket_t=SOCKET; static constexpr socket_t invalid_socket=INVALID_SOCKET;
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
using socket_t=int; static constexpr socket_t invalid_socket=-1;
#endif
namespace netask {
std::string tcp_request(const std::string& host,std::uint16_t port,const std::string& request) {
#ifdef _WIN32
    WSADATA w{}; if(WSAStartup(MAKEWORD(2,2),&w)!=0) throw std::runtime_error("Winsock startup failed");
#endif
    addrinfo hints{}; hints.ai_family=AF_UNSPEC; hints.ai_socktype=SOCK_STREAM; addrinfo* result=nullptr;
    auto service=std::to_string(port); int rc=getaddrinfo(host.c_str(),service.c_str(),&hints,&result);
    if(rc!=0) {
#ifdef _WIN32
        WSACleanup();
#endif
        throw std::runtime_error("host lookup failed"); }
    socket_t s=invalid_socket;
    for(auto* p=result;p;p=p->ai_next) { s=::socket(p->ai_family,p->ai_socktype,p->ai_protocol); if(s!=invalid_socket&&connect(s,p->ai_addr,static_cast<int>(p->ai_addrlen))==0) break;
#ifdef _WIN32
        if(s!=invalid_socket) closesocket(s);
#else
        if(s!=invalid_socket) close(s);
#endif
        s=invalid_socket;
    }
    freeaddrinfo(result); if(s==invalid_socket) {
#ifdef _WIN32
        WSACleanup();
#endif
        throw std::runtime_error("connection failed"); }
    auto msg=request+"\n"; std::size_t sent=0; while(sent<msg.size()) { int n=send(s,msg.data()+sent,static_cast<int>(msg.size()-sent),0); if(n<=0) throw std::runtime_error("send failed"); sent+=static_cast<std::size_t>(n); }
    std::string response; char buf[2048]; constexpr const char* end="\n.\n";
    while(response.find(end)==std::string::npos) { int n=recv(s,buf,sizeof(buf),0); if(n<=0) break; response.append(buf,static_cast<std::size_t>(n)); if(response.size()>131072) break; }
#ifdef _WIN32
    closesocket(s); WSACleanup();
#else
    close(s);
#endif
    if(response.empty()) throw std::runtime_error("server closed without a response");
    auto marker=response.find(end); if(marker!=std::string::npos) response.resize(marker);
    return response;
}
}
