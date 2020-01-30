#include "tunnel.hpp"
#include "syserr.hpp"
#include "NetLib.hpp"
#include <chrono>
#include <thread>

#include <rlib/scope_guard.hpp>

using namespace rlib;
using namespace std::chrono_literals;

void outbound_tunnel::initCString()
{
    //Resolve domain.
    boost::asio::ip::address servAddr;
    while(true) {
        try {
            servAddr = BoostNetLib::resolveDomainOnce(serverAddr);
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
        auto _conv_serverAddr = servAddr.to_v4().to_bytes();
        memcpy(conv_serverAddr, _conv_serverAddr.data(), 4);
    }
    if(servAddr.is_v6()) {
        auto _conv_serverAddr = servAddr.to_v6().to_bytes();
        memcpy(conv_serverAddr, _conv_serverAddr.data(), 16);
    }

    conv_passphrase.null_terminated = true;
    conv_passphrase.length = static_cast<uint32_t>(passphrase.size());
    conv_passphrase.str = (char *)calloc(1, passphrase.size() + 1);
    memcpy(conv_passphrase.str, passphrase.c_str(), passphrase.size());
}

void outbound_tunnel::destroyCString()
{
    free(conv_passphrase.str);
}

#ifndef WIN32
#include <arpa/inet.h>
//POSIX netwk
sockfd_t outbound_tunnel::newConnection() const
{
    addrinfo *paddr;
    addrinfo hints { 0 };
    sockfd_t sockfd;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    auto _ = getaddrinfo(serverAddr.c_str(), std::to_string(serverPort).c_str(), &hints, &paddr);
    if(_ != 0) sysdie("getaddrinfo failed. Check network connection to {}:{}; returnval={}, check `man getaddrinfo`'s return value."_format(serverAddr.c_str(), serverPort, _));
    rlib_defer([p=paddr]{freeaddrinfo(p);});

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
sockfd_t outbound_tunnel::newConnection() const
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
    auto _ = getaddrinfo(serverAddr.c_str(), std::to_string(serverPort).c_str(), &hints, &paddr);
    if(_ != 0) {
        WSACleanup();
        sysdie("getaddrinfo failed. Check network connection to {}:{}; returnval={}, check `man getaddrinfo`'s return value."_format(serverAddr.c_str(), serverPort, _));
    }
    rlib_defer([p=paddr]{WSACleanup();freeaddrinfo(p);});

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

bool outbound_tunnel::doAuth(connect_info *conn) const
{
    sockfd_t connfd = conn->connect_fd;
    // Send auth packet. client -> server
    binary_safe_string authdat = mod.client.makeAuthQuery(conn);
    rlib_defer([&]{std::free(authdat.str);});
    auto lengthBackup = authdat.length;
    if(lengthBackup > 64 * 1024 * 1024) die("authQuery is too long(incorrect)");
    authdat.length = htonl(authdat.length);
    rlog.debug("Authdat: length={}"_format(lengthBackup));
    sockIO::sendn_ex(connfd, &authdat.length, sizeof(authdat.length));
    sockIO::sendn_ex(connfd, authdat.str, lengthBackup);

    // Read server reply. server -> client
    uint32_t replyLen;
    sockIO::recvn_ex(connfd, &replyLen, sizeof(replyLen));
    replyLen = ntohl(replyLen);

    if(replyLen > 64 * 1024 * 1024) die("authReply is too long(incorrect)");
    char *rbuf = (char *)malloc(replyLen);
    rlib_defer([&]{std::free(rbuf);});
    sockIO::recvn_ex(connfd, rbuf, replyLen);

    return mod.client.onAuthReply(rbuf, replyLen, conn);
}

// It blocks, return true if auth success.
bool inbound_tunnel::doAuth(connect_info *conn) const
{
    sockfd_t connfd = conn->connect_fd;

    // Read client auth query. client -> server
    void *dataptr; uint32_t msgLen;
    std::tie(dataptr, msgLen) = NetLib::recv_length_and_packet(connfd);
    // rlib_defer([&]{std::free(dataptr);}); // mod.server.makeAuthReply will free this dataptr. This agreement may change in future.
    rlog.debug("receive client auth msg, len={}", msgLen);

    auto authMsg = binary_safe_string{false, msgLen, (char*)dataptr};

    // module process the auth.
    bool authFailed;
    auto authReplyMsg = mod.server.makeAuthReply(conn, authMsg, &authFailed);
    rlib_defer([&]{delete_binary_safe_string(authReplyMsg);});

    // Send auth reply. server -> client
    NetLib::send_length_and_packet(connfd, authReplyMsg.str, authReplyMsg.length);
    
    return !authFailed;
}

void inbound_tunnel::dealConnectionImpl(sockfd_t connfd) const
{
    connect_info myConnInfo{{0}, bindPort, make_binary_safe_string(passphrase), connfd, nullptr};
    sockfd_t nextHopConn = -1;
    if(!doAuth(&myConnInfo)) {
        const auto peername = rlib::get_peer_name(connfd);
        die("client {}:{} auth failed."_format(peername.first, peername.second));
    }

    rlib_defer([&]{mod.server.onCloseConn(&myConnInfo);});

    // data exchange begin
    //
    fd_set rset;
    FD_ZERO(&rset);
    while(true) {
        FD_SET(connfd, &rset);
        if(nextHopConn != -1) FD_SET(nextHopConn, &rset);
        if (-1 == select(std::max(connfd, nextHopConn) + 1, &rset, NULL, NULL, NULL))
            sysdie("Select failed");
        if (FD_ISSET(connfd, &rset)) {
            // Client data.
            void *dataptr; uint32_t msgLen;
            std::tie(dataptr, msgLen) = NetLib::recv_length_and_packet(connfd);
            // TODO: detect connection closing, and do not throw exception on it.
            // mod.server.decode will free this "dataptr". remove this behavior in the future.
            rlog.debug("Got data from skcli, length={}", msgLen);
            // decode this packet.
            auto thisQuery = mod.server.decode(&myConnInfo, binary_safe_string{false, msgLen, (char*)dataptr});

            if(NetLib::is_dns_query(thisQuery)) {
                // DNS packet. No 'nextHopConn' required. //////////////////////////////////////////////////////////////////
                auto domainStr = NetLib::get_domain_from_dns_query(thisQuery);
                rlog.debug("Previous skcli data is dns packet, domain={}, working on that.", domainStr);
                auto nextHopIp = BoostNetLib::resolveDomainOnce(domainStr);

                // Prepare response.
                client_query thisResponse {{0}, thisQuery.destination_port, binary_safe_string{false, 0, NULL}};

                if(nextHopIp.is_v4()) {
                    auto _conv_serverAddr = nextHopIp.to_v4().to_bytes();
                    rlog.debug("Previous skcli data is dns packet, length={}, domain={}, resolved to IPv4 array: {}.", msgLen, domainStr, NetLib::printData(_conv_serverAddr.data(), 4));
                    memcpy(thisResponse.destination_ip, _conv_serverAddr.data(), 4);
                }
                if(nextHopIp.is_v6()) {
                    auto _conv_serverAddr = nextHopIp.to_v6().to_bytes();
                    rlog.debug("Previous skcli data is dns packet, length={}, domain={}, resolved to IPv6 array: {}.", msgLen, domainStr, NetLib::printData(_conv_serverAddr.data(), 16));
                    memcpy(thisResponse.destination_ip, _conv_serverAddr.data(), 16);
                }

                // Send this response back.
                auto encodedthisResponse = mod.server.encode(&myConnInfo, thisResponse);
                rlib_defer([&]{std::free(encodedthisResponse.str);});
                NetLib::send_length_and_packet(connfd, encodedthisResponse.str, encodedthisResponse.length);
            }
            else {
                // data packet. Create connection if 'nextHopConn' isn't present. /////////////////////////////////////////
                if(nextHopConn == -1) {
                    auto nextHopAddr = NetLib::ip_array_to_str(thisQuery.destination_ip);
                    rlog.verbose("New connection to {}:{}", nextHopAddr, thisQuery.destination_port);
                    nextHopConn = rlib::quick_connect(nextHopAddr, thisQuery.destination_port);
                }
                // pass the data to nextHop now. Protocol=FREEDOM.
                rlib::sockIO::sendn_ex(nextHopConn, thisQuery.payload.str, thisQuery.payload.length);
            }
        }
        if (nextHopConn != -1 && FD_ISSET(nextHopConn, &rset)) {
            // nextHop data. /////////////////////////////////////////////////////////////////////////////////////////////
            auto freedomData = rlib::sockIO::quick_recvall(nextHopConn);
            if(freedomData.empty()) return; // Connection closed.
            rlog.debug("Got data from nextHop, length={}", freedomData.size());
            client_query cq {{0}, bindPort, make_binary_safe_string(freedomData)};
            // mod.server.encode will free the "malloc-ed" binary_safe_string. DO NOT USE freedomData.data() which causes double-free.
            // TODO: Change this behavior in the future.
            auto encodedData = mod.server.encode(&myConnInfo, cq);

            NetLib::send_length_and_packet(connfd, encodedData.str, encodedData.length);
        }
    }
}

void inbound_tunnel::dealConnection(sockfd_t connfd) const {
    rlog.info("Connection {} accepted.", connfd);
    try {
        dealConnectionImpl(connfd);
    }
    catch(std::exception &e) {
        rlog.error("Connection {} caught exception: {}", connfd, e.what());
    }
    rlog.info("Connection {} closed.", connfd);
    close(connfd);
}

[[noreturn]] void inbound_tunnel::listen() const
{
    auto listenfd = rlib::quick_listen(bindAddr, bindPort);
    rlog.info("Snakesocks listening {}:{} ...", bindAddr, bindPort);
#ifndef ___NO_THREAD_POOL
    ThreadPool pool;
#endif
    while(true) {
        auto connfd = rlib::quick_accept(listenfd);
        rlog.debug("new client conn, connfd={}", connfd);
#ifdef ___NO_THREAD_POOL
        std::thread(&inbound_tunnel::dealConnection, this, connfd).detach();
#else
        pool.addTask(std::bind(&inbound_tunnel::dealConnection, this, connfd));
#endif
    }



}
