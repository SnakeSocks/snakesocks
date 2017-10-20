#ifndef SNAKESOCKS_SOCKS5_HPP
#define SNAKESOCKS_SOCKS5_HPP

#include "tunnel.hpp"

#include <string>
//#include <boost/core/noncopyable.hpp>
#include "noncopyable.hpp"
#include "connection.hpp"
using std::string;

#ifndef WIN32
using fd=int;
#else
#include <winsock2.h>
using fd=SOCKET;
#endif

class Socks5Server : private rlib::noncopyable
{
public:
    Socks5Server() = delete;
    Socks5Server(const string &ip, uint16_t port, tunnel &&sks) : bindIp(ip), bindPort(port), skserver(std::move(sks)){}
    [[noreturn]] void listen(); //will block current thread.
    tunnel skserver;
private:
    void dealConnection(fd connfd);
    string bindIp;
    uint16_t bindPort;
};

class Socks5Connection : private rlib::noncopyable
{
public:
    Socks5Connection() = delete;
    Socks5Connection(const Socks5Server &server, fd acceptedConnFd) : m_server(server), m_fd(acceptedConnFd) {}
    ~Socks5Connection() {close(m_fd);}
    void launch()
    {
        try
        {
            handshake_pkgs();
            auto rawAddr = connect_pkgs();
            SnakeConnection nextHop(m_server.skserver);
            auto templateQueryWithIp = dns_pkgs(nextHop, rawAddr);
            passData(nextHop, templateQueryWithIp);
        }
        catch(std::exception &e)
        {
            LOG(2) << "Exception caught from child thread:" << e.what() << std::endl;
        }
        LOGF(1)("Socks5: Connection %d closed.\n", m_fd);
    }
private:
    static auto unpackConnectionPacket(const char *pkgStr);
    void handshake_pkgs();
    client_query connect_pkgs(); //return a client_query
    client_query dns_pkgs(SnakeConnection &nextHop, client_query &rawAddr);
    void passData(SnakeConnection &nextHop, const client_query &templateQueryWithIp); //This function is not abstracted, so this header is not designed as a library....

    fd m_fd;
    const Socks5Server &m_server;
};


#endif //SNAKESOCKS_SOCKS5_HPP
