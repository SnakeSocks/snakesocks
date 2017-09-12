#include "tunnel.hpp"
#include "syserr.hpp"
#include "sio.hpp"
#include "scope_guard.hpp"
#include "NetLib.hpp"

using namespace rlib;

void tunnel::initCString()
{
    //Resolve domain.
    auto servAddr = BoostNetLib::resolveDomainOnce(serverIp);
    if(servAddr.is_v4()) {
        auto _conv_serverIp = servAddr.to_v4().to_bytes();
        memcpy(conv_serverIp, _conv_serverIp.data(), 4);
    }
    if(servAddr.is_v6()) {
        auto _conv_serverIp = servAddr.to_v6().to_bytes();
        memcpy(conv_serverIp, _conv_serverIp.data(), 16);
    }

    conv_passphrase.null_terminated = true;
    conv_passphrase.length = static_cast<uint32_t>(passphrase.size());
    conv_passphrase.str = (char *)calloc(1, passphrase.size() + 1);
    memcpy(conv_passphrase.str, passphrase.c_str(), passphrase.size());
}

void tunnel::destroyCString()
{
    free(conv_passphrase.str);
}

#ifndef WIN32
#include <arpa/inet.h>
//POSIX netwk
fd tunnel::newConnection() const
{
    addrinfo *paddr;
    addrinfo hints { 0 };
    fd sockfd;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    auto _ = getaddrinfo(serverIp.c_str(), std::to_string(serverPort).c_str(), &hints, &paddr);
    if(_ != 0) sysdie("getaddrinfo failed. Check network connection to %s:%d; returnval=%d, check `man getaddrinfo`'s return value.", serverIp.c_str(), serverPort, _);
    defer([p=paddr](){freeaddrinfo(p);});

    bool success = false;
    for (addrinfo *rp = paddr; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1)
            continue;
        int reuse = 1;
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(int)) < 0) sysdie("setsockopt(SO_REUSEADDR) failed");
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(int)) < 0) sysdie("setsockopt(SO_REUSEPORT) failed");
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            success = true;
            break; /* Success */
        }
        close(sockfd);
    }
    if(!success) sysdie("Failed to connect to any of these addr.");

    return sockfd;
}
#else
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
//Winsock API
fd tunnel::newConnection() const
{
    WSADATA wsaData;
    SOCKET sockfd = INVALID_SOCKET;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) sysdie("WSAStartup failed with error: %d\n", iResult);
        
    addrinfo *paddr;
    addrinfo hints { 0 };

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    auto _ = getaddrinfo(serverIp.c_str(), std::to_string(serverPort).c_str(), &hints, &paddr);
    if(_ != 0) {
        WSACleanup();
        sysdie("getaddrinfo failed. Check network connection to %s:%d; returnval=%d, check `man getaddrinfo`'s return value.", serverIp.c_str(), serverPort, _); 
    }   
    defer([p=paddr](){WSACleanup();freeaddrinfo(p);});

    bool success = false;
    for (addrinfo *rp = paddr; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == INVALID_SOCKET)
            continue;
        int reuse = 1;
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(int)) < 0) sysdie("setsockopt(SO_REUSEADDR) failed");
        //if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(int)) < 0) sysdie("setsockopt(SO_REUSEPORT) failed");
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != SOCKET_ERROR) {
            success = true;
            break; /* Success */
        }
        closesocket(sockfd);
    }   
    if(!success) sysdie("Failed to connect to any of these addr.");

    return sockfd;
}
#endif

bool tunnel::doAuth(connect_info *conn) const
{
    fd connfd = conn->connect_fd;
    binary_safe_string authdat = mod.makeAuthQuery(conn);
    defer([&](){std::free(authdat.str);});
    auto lengthBackup = authdat.length;
    authdat.length = htonl(authdat.length);
    debug(3) printf("Authdat: length=%u\n", lengthBackup);
    if(-1 == fdIO::writen(connfd, &authdat.length, sizeof(authdat.length))) sysdie("Write failed.");
    if(-1 == fdIO::writen(connfd, authdat.str, lengthBackup)) sysdie("Write failed.");

    uint32_t replyLen;
    if(fdIO::readn(connfd, &replyLen, 4) == -1) sysdie("readn failed.");
    replyLen = ntohl(replyLen);

    if(replyLen > 1024 * 1024 * 1024) sysdie("Fatal: memory out of range. ");
    char *rbuf = (char *)malloc(replyLen);
    defer([&](){std::free(rbuf);});
    if(-1 == fdIO::readn(connfd, rbuf, replyLen)) sysdie("read failed.");

    return mod.onAuthReply(rbuf, replyLen, conn);
}
