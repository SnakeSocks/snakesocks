#include "socks5.hpp"

#include "syserr.hpp"
#include <rlib/sys/sio.hpp>
#include <rlib/scope_guard.hpp>
#include "NetLib.hpp"
#include "threadPool.hpp"
#include "connection.hpp"

#include <functional>
#include <thread>
#include <string>
#include <iostream>
#include <csignal>
#include <cstdlib>

#include <boost/asio.hpp>

using std::string;
using std::cout;
using std::endl;

#ifndef WIN32
using fd=int;
#else
#include <winsock2.h>
using fd=SOCKET;
#endif

using namespace rlib;

void std_string_to_bin_safe(const string &src, binary_safe_string &dst)
{
    dst.length = src.size();
    dst.null_terminated = false;
    dst.str = (char *) malloc(src.size());
    memcpy(dst.str, src.data(), src.size());
}

void Socks5Server::dealConnection(fd connfd) {
    Socks5Connection(*this, connfd).launch();
}

#ifndef WIN32
//POSIX impl
[[noreturn]] void Socks5Server::listen()
{
    addrinfo *psaddr;
    addrinfo hints{0};
    fd listenfd;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    auto _ = getaddrinfo(bindIp.c_str(), std::to_string(bindPort).c_str(), &hints, &psaddr);
    if (_ != 0) sysdie("Failed to getaddrinfo. returnval=%d, check `man getaddrinfo`'s return value.", _);

    bool success = false;
    for (addrinfo *rp = psaddr; rp != NULL; rp = rp->ai_next)
    {
        listenfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (listenfd == -1)
            continue;
        int reuse = 1;
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse, sizeof(int)) < 0)
            sysdie("setsockopt(SO_REUSEADDR) failed");
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, (const char *) &reuse, sizeof(int)) < 0)
            sysdie("setsockopt(SO_REUSEPORT) failed");
        if (bind(listenfd, rp->ai_addr, rp->ai_addrlen) == 0)
        {
            success = true;
            break; /* Success */
        }
        close(listenfd);
    }
    if (!success) sysdie("Failed to bind to any of these addr.");

    if (-1 == ::listen(listenfd, 16)) sysdie("listen failed.");

    cout << "Listening on " << bindIp << ":" << bindPort << endl;
#ifndef ___NO_THREAD_POOL
    ThreadPool pool;
#endif
    while (true)
    {
        fd connfd = accept(listenfd, nullptr, nullptr);
        if (connfd == -1) continue;
#ifdef ___NO_THREAD_POOL
        std::thread(&Socks5Server::dealConnection, this, connfd).detach();
#else
        pool.addTask(std::bind(&Socks5Server::dealConnection, this, connfd));
#endif
    }
    freeaddrinfo(psaddr);
    sysdie("Noreturn function returns.");
}

#else

#include <winsock2.h>
#include <ws2tcpip.h>
//Winsock impl
[[noreturn]] void s5server::run()
{
    WSADATA wsaData;
    SOCKET listenfd = INVALID_SOCKET;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) sysdie("WSAStartup failed with error: %d\n", iResult);

    addrinfo *psaddr;
    addrinfo hints { 0 };
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = IPPROTO_TCP;
    auto _ = getaddrinfo(bindIp.c_str(), std::to_string(bindPort).c_str(), &hints, &psaddr);
    if(_ != 0) {
        WSACleanup();
        sysdie("Failed to getaddrinfo. returnval=%d, check `man getaddrinfo`'s return value.", _);
    }

    bool success = false;
    for (addrinfo *rp = psaddr; rp != NULL; rp = rp->ai_next) {
        listenfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (listenfd == INVALID_SOCKET)
            continue;
        int reuse = 1;
        if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(int)) < 0) sysdie("setsockopt(SO_REUSEADDR) failed");
        //if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(int)) < 0) sysdie("setsockopt(SO_REUSEPORT) failed");
        if (bind(listenfd, rp->ai_addr, rp->ai_addrlen) != SOCKET_ERROR) {
            success = true;
            break; /* Success */
        }
        closesocket(listenfd);
    }
    if(!success) sysdie("Failed to bind to any of these addr.");

    if(SOCKET_ERROR == ::listen(listenfd, 16)) sysdie("listen failed.");

    cout << "Listening on " << bindIp << ":" << bindPort << endl;
#ifndef ___NO_THREAD_POOL
    ThreadPool pool;
#endif
    while(true)
    {
        SOCKET connfd = accept(listenfd, nullptr, nullptr);
        if(connfd == INVALID_SOCKET) continue;

#ifdef ___NO_THREAD_POOL
        std::thread(&Socks5Server::dealConnection, this, connfd).detach();
#else
        pool.addTask(std::bind(&Socks5Server::dealConnection, this, connfd));
#endif
    }
    freeaddrinfo(psaddr);
    WSACleanup();
    sysdie("Returning from nonreturn function.");
}
#endif

auto Socks5Connection::unpackConnectionPacket(const char *pkgStr)
{ //remoteInfo.dstIp is deprecated. remoteInfo.payload contains remoteAddr, and must be processed by dns. It's a mallocd bin_str.
    if (pkgStr[0] != 5) die("Broken package. Incorrect ver.");
    char CMD = pkgStr[1];
    char ATYP = pkgStr[3];
    client_query remoteInfo;
    memset(remoteInfo.destination_ip, 0, 16);
    if (ATYP == 1)
    {
        //memcpy(remoteInfo.destination_ip, pkgStr + 4, 4);
        std::array<unsigned char, 4> bArr;
        memcpy(bArr.data(), pkgStr + 4, 4);
        string v4addr = boost::asio::ip::address_v4(bArr).to_string();
        std_string_to_bin_safe(v4addr, remoteInfo.payload);

        remoteInfo.destination_port = *reinterpret_cast<const uint16_t *>(&pkgStr[8]);
    } else if (ATYP == 4)
    {
        //memcpy(remoteInfo.destination_ip, pkgStr + 4, 16);
        std::array<unsigned char, 16> bArr;
        memcpy(bArr.data(), pkgStr + 4, 16);
        string v6addr = boost::asio::ip::address_v6(bArr).to_string();
        std_string_to_bin_safe(v6addr, remoteInfo.payload);
        remoteInfo.destination_port = *reinterpret_cast<const uint16_t *>(&pkgStr[20]);
    } else if (ATYP == 3)
    {
        size_t addrlen = (size_t) pkgStr[4];
        remoteInfo.destination_port = *reinterpret_cast<const uint16_t *>(&pkgStr[5 + addrlen]);

        remoteInfo.payload.length = addrlen;
        remoteInfo.payload.null_terminated = false;
        remoteInfo.payload.str = (char *) malloc(addrlen);
        memcpy(remoteInfo.payload.str, pkgStr + 5, addrlen);
    } else
        die("Broken package.Invalid ATYP");

    remoteInfo.destination_port = htons(remoteInfo.destination_port);
    return std::make_tuple(CMD, ATYP, remoteInfo);
}

void Socks5Connection::handshake_pkgs()
{
    char headBuf[258];
    if (fdIO::readn(m_fd, headBuf, 2) == -1) sysdie("readn socket failed");

    if (headBuf[0] != (char) 5) die("Broken protocol. (Incorrect protocol number)");
    size_t methodSize = (size_t) headBuf[1];

    if (fdIO::readn(m_fd, headBuf + 2, methodSize) == -1) sysdie("readn socket failed");

    bool methodOk = false;
    for (size_t cter = 0; cter < methodSize; ++cter)
    {
        if (headBuf[2 + cter] == 0)
        {
            methodOk = true;
            break;
        }
    }
    headBuf[0] = 5;
    headBuf[1] = 0;
    if (!methodOk) headBuf[1] = (char) 255;
    if (fdIO::writen(m_fd, headBuf, 2) == -1) sysdie("Failed to write socket.");
}

client_query Socks5Connection::connect_pkgs()
{
    char buf[263];
    if (fdIO::readn(m_fd, buf, 5) == -1) sysdie("readn failed.");
    size_t addrlen = 0;
    bool useDomain = false;
    switch (buf[3])
    {
        case 1:
            addrlen = 4;
            break;
        case 3:
            addrlen = (size_t) buf[4] + 1; // mogic logic.
            useDomain = true;
            break;
        case 4:
            addrlen = 16;
            break;
        default:
            die("Wrong ATYP in connect packet.");
    }
    if (fdIO::readn(m_fd, buf + 5, addrlen + 1) == -1) sysdie("readn failed.");

    char CMD, ATYP;
    client_query remoteInfo;
    std::tie(CMD, ATYP, remoteInfo) = unpackConnectionPacket(buf);
    rlib::scope_guard remoteInfoGuarder = [&](){free(remoteInfo.payload.str);};
    if (CMD != 01)
        die("Request can not be processed.(CMD=%d)", (int) CMD);

    //Make success reply.
    buf[1] = 0;
    auto pkgLen = addrlen + 6;
    fdIO::writen_ex(m_fd, buf, pkgLen);

    remoteInfoGuarder.dismiss();
    return remoteInfo;
}

#define max(a, b) ((a)>(b)?(a):(b))

/*
#include <chrono>
#include <iomanip>
void printTime()
{
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::cout << std::put_time(std::localtime(&now_c), "%c") << std::endl;
}
*/

client_query Socks5Connection::dns_pkgs(SnakeConnection &nextHop, client_query &rawAddr)
{
    // Send first data pack (DNS)
    binary_safe_string firstPack = m_server.skserver.mod.encode(binary_safe_string{false, 0, nullptr}, nextHop.GetConnInfo(), rawAddr);
    defer([&](){std::free(firstPack.str);});
    auto lengthBackup = firstPack.length;
    firstPack.length = htonl(firstPack.length);
    LOG(3) << "First data pack to send: len=" << lengthBackup << std::endl;
    nextHop.strict_sendn(&firstPack.length, sizeof(firstPack.length));
    nextHop.strict_sendn(firstPack.str, lengthBackup);

    // Get DNS responce.
    binary_safe_string firstReturnPack;
    nextHop.strict_recvn(&firstReturnPack.length, sizeof(firstReturnPack.length));
    firstReturnPack.length = ntohl(firstReturnPack.length);
    if(firstReturnPack.length == 0xff7f0000) //Unknown error.
    {
        std::printf("Conn%d:Fatal error prevented. Unknown packet 0xff7f0000 appeared after we solved this bug. Please report!!!\n", m_fd);
        abort();
    }
    firstReturnPack.str = (char *) malloc(firstReturnPack.length);
    nextHop.strict_recvn(firstReturnPack.str, firstReturnPack.length);
    client_query realTemplate = m_server.skserver.mod.decode(firstReturnPack, nextHop.GetConnInfo());

    // First data pack done.
    return realTemplate;
}

void Socks5Connection::passData(SnakeConnection &nextHop, const client_query &templateQueryWithIp)
{
    fd_set rset;
    FD_ZERO(&rset);
    while (true)
    {
        FD_SET(m_fd, &rset);
        FD_SET(nextHop.GetConnFd(), &rset);
        if (-1 == select(max(m_fd, nextHop.GetConnFd()) + 1, &rset, NULL, NULL, NULL))
            sysdie("Select failed");
        if (FD_ISSET(m_fd, &rset))
        {
            //read from thisHop and write to nextHop
            void *bridgeBuffer = NULL;
            auto ret = rlib::fdIO::readall(m_fd, &bridgeBuffer, 0);
            if(ret <= 0)
                free(bridgeBuffer);
            if (ret == 0)
                goto _close_conn;
            else if(ret < 0)
                sysdie("read failed");
            auto encodedDat = m_server.skserver.mod.encode(binary_safe_string{false, static_cast<uint32_t>(ret), (char *) bridgeBuffer}, nextHop.GetConnInfo(), templateQueryWithIp);
            defer([&](){std::free(encodedDat.str);});
            auto lengthBackup = encodedDat.length;
            encodedDat.length = htonl(encodedDat.length);
            LOG(3) << "Data pack to send: len=" << lengthBackup << std::endl;
            LOG(4) << NetLib::printData(encodedDat.str, lengthBackup) << std::endl;
            nextHop.strict_sendn(&encodedDat.length, sizeof(encodedDat.length));
            nextHop.strict_sendn(encodedDat.str, lengthBackup);
        }
        if (FD_ISSET(nextHop.GetConnFd(), &rset))
        {
            //read from nextHop and write to thisHop
            uint32_t binLen = 0;
            nextHop.recvn(&binLen, 4);
            binLen = ntohl(binLen);
            if (binLen == 0) goto _close_conn;
            void *bridgeBuffer = malloc(binLen);
            nextHop.recvn_and_free_on_err(bridgeBuffer, binLen);
            LOG(3) << "Data pack recv:len=" << binLen << std::endl;
            LOG(4) << NetLib::printData(bridgeBuffer, binLen) << std::endl;
            auto decodedDat = m_server.skserver.mod.decode(binary_safe_string{false, binLen, (char *) bridgeBuffer}, nextHop.GetConnInfo()).payload;
            defer([&](){free(decodedDat.str);});
            if(sockIO::sendn(m_fd, decodedDat.str, decodedDat.length, MSG_NOSIGNAL) == -1)
                sysdie("write failed");
        }
    }
    _close_conn:;
}
