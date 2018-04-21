#include "tunnel.hpp"
#include "syserr.hpp"
#include <rlib/sys/sio.hpp>
#include "NetLib.hpp"
#include <chrono>
#include <thread>

//Import lib with polluting macro at last.. (macro defer)
#include <rlib/scope_guard.hpp>

using namespace rlib;
using namespace std::chrono_literals;

void tunnel::initCString()
{
    //Resolve domain.
    boost::asio::ip::address servAddr;
    while(true) {
        try {
            servAddr = BoostNetLib::resolveDomainOnce(serverIp);
            break;
        }
        catch(boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::system::system_error> >) {
            rlog.error("Can not resolve domain {}."_format(servAddr));
            if(nx_retry)
                std::this_thread::sleep_for(15s);
            else
                throw;
        }
    }

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
    if(_ != 0) sysdie("getaddrinfo failed. Check network connection to {}:{}; returnval={}, check `man getaddrinfo`'s return value."_format(serverIp.c_str(), serverPort, _));
    defer([p=paddr]{freeaddrinfo(p);});

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
    if (iResult != 0) sysdie("WSAStartup failed with error: {}"_format(iResult));
 
    addrinfo *paddr;
    addrinfo hints { 0 };

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    auto _ = getaddrinfo(serverIp.c_str(), std::to_string(serverPort).c_str(), &hints, &paddr);
    if(_ != 0) {
        WSACleanup();
        sysdie("getaddrinfo failed. Check network connection to {}:{}; returnval={}, check `man getaddrinfo`'s return value."_format(serverIp.c_str(), serverPort, _));
    }
    defer([p=paddr]{WSACleanup();freeaddrinfo(p);});

    bool success = false;
    for (addrinfo *rp = paddr; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == INVALID_SOCKET)
            continue;
        int reuse = 1;
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(int)) < 0) sysdie("setsockopt(SO_REUSEADDR) failed");
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
    defer([&]{std::free(authdat.str);});
    auto lengthBackup = authdat.length;
    if(lengthBackup > 64 * 1024 * 1024) die("Reply is too long(incorrect)");
    authdat.length = htonl(authdat.length);
    rlog.debug("Authdat: length={}"_format(lengthBackup));
    fdIO::writen_ex(connfd, &authdat.length, sizeof(authdat.length));
    fdIO::writen_ex(connfd, authdat.str, lengthBackup);

    uint32_t replyLen;
    fdIO::readn_ex(connfd, &replyLen, 4);
    replyLen = ntohl(replyLen);

    if(replyLen > 64 * 1024 * 1024) die("Reply is too long(incorrect)");
    char *rbuf = (char *)malloc(replyLen);
    defer([&]{std::free(rbuf);});
    fdIO::readn_ex(connfd, rbuf, replyLen);

    return mod.onAuthReply(rbuf, replyLen, conn);
}
