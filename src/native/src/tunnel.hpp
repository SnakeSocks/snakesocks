#ifndef SNAKESOCKS_TUNNEL_HPP
#define SNAKESOCKS_TUNNEL_HPP

#include "module.hpp"
#include "debug.hpp"

#include <string>
#include <rlib/class_decorator.hpp>
using std::string;

#include <rlib/sys/sio.hpp>

class outbound_tunnel : private rlib::noncopyable
{
public:
    outbound_tunnel() = delete;
    outbound_tunnel(const string &addr, uint16_t port, const string &pswdFromCfg, module_interface &&cmod, bool nx_retry)
            : serverAddr(addr), serverPort(port), passphrase(pswdFromCfg), mod(std::move(cmod)), nx_retry(nx_retry) {initCString();}
    outbound_tunnel(outbound_tunnel &&ano)
            : mod(std::move(ano.mod)), serverAddr(std::move(ano.serverAddr)),
              passphrase(std::move(ano.passphrase)), serverPort(ano.serverPort) {initCString();}
    ~outbound_tunnel() {destroyCString();}
    sockfd_t newConnection() const; //Return a new connfd after authorization. If auth failed, return -1;
public:
    bool doAuth(connect_info *) const;
    module_interface mod;
public:
    string serverAddr;
    uint16_t serverPort;
    string passphrase;
    bool nx_retry;

    uint8_t conv_serverAddr[16] = { 0 };
    binary_safe_string conv_passphrase;
private:
    void initCString();
    void destroyCString();
};

class inbound_tunnel : private rlib::noncopyable
{
public:
    inbound_tunnel() = delete;
    inbound_tunnel(const string &bindAddr, const uint16_t bindPort, const string &passphrase, module_interface &&mod)
        : bindAddr(bindAddr), bindPort(bindPort), passphrase(passphrase), mod(std::move(mod)) {}
    [[noreturn]] void listen() const;
private:
    bool doAuth(connect_info *) const;
    void dealConnectionImpl(sockfd_t connfd) const;
    void dealConnection(sockfd_t connfd) const;
    module_interface mod;

    string bindAddr;
    uint16_t bindPort;
    string passphrase;
};


#endif //SNAKESOCKS_TUNNEL_HPP
