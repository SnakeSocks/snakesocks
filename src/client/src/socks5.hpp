#ifndef SNAKESOCKS_SOCKS5_HPP
#define SNAKESOCKS_SOCKS5_HPP

#include "tunnel.hpp"

#include <string>
//#include <boost/core/noncopyable.hpp>
#include "noncopyable.hpp"
using std::string;

#ifndef WIN32
using fd=int;
#else
#include <winsock2.h>
using fd=SOCKET;
#endif

class s5server : private rlib::noncopyable
{
public:
    s5server() = delete;
    s5server(const string &ip, uint16_t port, tunnel &&sss) : bindIp(ip), bindPort(port), ssserver(std::move(sss)){RECORD}
    [[noreturn]] void run(); //sync
private:
    auto unpackConnectionPacket(const char *pkgStr);
    void handshake(fd connfd);
    client_query doConnect(fd connfd); //return a client_query
    void passPackets(fd connfd, client_query);
    void _dealConnection(fd connfd);
    void dealConnection(fd connfd);
    tunnel ssserver; //I need to get fd_nexthop from ssserver.newConnection
    string bindIp;
    uint16_t bindPort;
};


#endif //SNAKESOCKS_SOCKS5_HPP
