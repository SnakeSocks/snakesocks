#ifndef SNAKESOCKSCLI_CONNECTION_HPP
#define SNAKESOCKSCLI_CONNECTION_HPP

#include "module.hpp"
#include "debug.hpp"
#include "tunnel.hpp"
#include "syserr.hpp"

#include <string>
#include <rlib/sys/sio.hpp>
#include <rlib/class_decorator.hpp>

using std::string;

#include <rlib/sys/sio.hpp>

class SnakeConnection : private rlib::noncopyable
{
public:
    SnakeConnection() = delete;
    SnakeConnection(const outbound_tunnel &server)
            : mySnakeServer(server) {
        initConnInfo();
        connect();
    }
    ~SnakeConnection() {
        if(myConnInfo) {
            mySnakeServer.mod.client.onCloseConn(myConnInfo);
            delete myConnInfo;
        }
        if(connfd) close(connfd);
    }
    connect_info *GetConnInfo(){
        return myConnInfo;
    }
    sockfd_t GetConnFd() {
        return myConnInfo->connect_fd;
    }

    void sendn(const void *data, size_t len) {
        rlib::sockIO::sendn_ex(connfd, (const char *)data, len, MSG_NOSIGNAL);
    }
    void recvn(void *data, size_t len) {
        rlib::sockIO::recvn_ex(connfd, (char *)data, len, MSG_NOSIGNAL);
    }

    void recvn_and_free_on_err(void *data, size_t len) {
        auto ret = rlib::sockIO::recvn(connfd, (char *)data, len, MSG_NOSIGNAL);
        if(ret == -1) { 
            free(data);
            sysdie("RecvnAndFree failed.");
        }
    }

private:
    const outbound_tunnel &mySnakeServer;
    connect_info *myConnInfo = nullptr;
    sockfd_t connfd = 0;
    void connect() {
        connfd = mySnakeServer.newConnection();
        myConnInfo->connect_fd = connfd;
        if(!mySnakeServer.doAuth(myConnInfo))
            die("Auth failed.");
    }
    void initConnInfo() {
        myConnInfo = new connect_info{{0}, mySnakeServer.serverPort, mySnakeServer.conv_passphrase, 0, nullptr};
        memcpy(myConnInfo->server_ip, mySnakeServer.conv_serverAddr, 16);
    }
};




#endif //SNAKESOCKSCLI_CONNECTION_HPP
