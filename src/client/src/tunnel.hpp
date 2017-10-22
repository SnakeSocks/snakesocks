#ifndef SNAKESOCKS_TUNNEL_HPP
#define SNAKESOCKS_TUNNEL_HPP

#include "module.hpp"
#include "debug.hpp"

#include <string>
#include <rlib/noncopyable.hpp>
using std::string;

#ifndef WIN32
using fd=int;
#else
#include <winsock2.h>
using fd=SOCKET;
#endif

class tunnel : private rlib::noncopyable
{
public:
    tunnel() = delete;
    tunnel(const string &ip, uint16_t port, const string &psFromCfg, client_module &&cmod)
            : serverIp(ip), serverPort(port), passphrase(psFromCfg) ,mod(std::move(cmod)) {initCString();}
    tunnel(tunnel &&ano)
            : mod(std::move(ano.mod)), serverIp(std::move(ano.serverIp)),
              passphrase(std::move(ano.passphrase)), serverPort(ano.serverPort) {initCString();}
    ~tunnel() {destroyCString();}
    fd newConnection() const; //Return a new connfd after authorization. If auth failed, return -1;
public:
    bool doAuth(connect_info *) const;
    client_module mod;
public:
    string serverIp;
    uint16_t serverPort;
    string passphrase;
    uint8_t conv_serverIp[16] = { 0 };
    binary_safe_string conv_passphrase;
private:
    void initCString();
    void destroyCString();
};


#endif //SNAKESOCKS_TUNNEL_HPP
