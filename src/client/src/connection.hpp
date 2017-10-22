#ifndef SNAKESOCKSCLI_CONNECTION_HPP
#define SNAKESOCKSCLI_CONNECTION_HPP

#include "module.hpp"
#include "debug.hpp"
#include "tunnel.hpp"
#include "syserr.hpp"

#include <string>
#include <rlib/sys/sio.hpp>
#include <rlib/noncopyable.hpp>

using std::string;

#ifndef WIN32
using fd=int;
#else
#include <winsock2.h>
using fd=SOCKET;
#endif

class SnakeConnection : private rlib::noncopyable
{
public:
    SnakeConnection() = delete;
    SnakeConnection(const tunnel &server)
            : mySnakeServer(server) {
        initConnInfo();
        connect();
    }
    ~SnakeConnection() {
        if(myConnInfo) {
            mySnakeServer.mod.on_close_conn(myConnInfo);
            delete myConnInfo;
        }
        if(connfd) close(connfd);
    }
    connect_info *GetConnInfo(){
        return myConnInfo;
    }
    fd GetConnFd() {
        return myConnInfo->connect_fd;
    }

    void sendn(const void *data, size_t len) {
        rlib::sockIO::sendn_ex(connfd, (const char *)data, len, MSG_NOSIGNAL);
    }
    void recvn(void *data, size_t len) {
        rlib::sockIO::recvn_ex(connfd, (char *)data, len, MSG_NOSIGNAL);
    }

    void strict_sendn(const void *data, size_t len) {
        if(rlib::sockIO::sendn_ex(connfd, (const char *)data, len, MSG_NOSIGNAL) < len)
            throw std::runtime_error("invalid sendn");
    }
    void strict_recvn(void *data, size_t len) {
        if(rlib::sockIO::recvn_ex(connfd, (char *)data, len, MSG_NOSIGNAL) < len)
            throw std::runtime_error("invalid recvn");
    }

    void sendn_and_free_on_err(const void *data, size_t len) {
        auto ret = rlib::sockIO::sendn(connfd, (const char *)data, len, MSG_NOSIGNAL);
        if(ret == -1) {
            free((void *)data);
            sysdie("SendnAndFree failed.");
        } 
    }
    void recvn_and_free_on_err(void *data, size_t len) {
        auto ret = rlib::sockIO::recvn(connfd, (char *)data, len, MSG_NOSIGNAL);
        if(ret == -1) { 
            free(data);
            sysdie("RecvnAndFree failed.");
        }
    }

private:
    const tunnel &mySnakeServer;
    connect_info *myConnInfo = nullptr;
    fd connfd = 0;
    void connect() {
        connfd = mySnakeServer.newConnection();
        myConnInfo->connect_fd = connfd;
        if(!mySnakeServer.doAuth(myConnInfo))
            die("Auth failed.");
    }
    void initConnInfo() {
        myConnInfo = new connect_info{{0}, mySnakeServer.serverPort, mySnakeServer.conv_passphrase, 0, nullptr};
        memcpy(myConnInfo->server_ip, mySnakeServer.conv_serverIp, 16);
    }
};




#endif //SNAKESOCKSCLI_CONNECTION_HPP
