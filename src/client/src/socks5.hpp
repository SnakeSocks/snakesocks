#ifndef SNAKESOCKS_SOCKS5_HPP
#define SNAKESOCKS_SOCKS5_HPP

#include "tunnel.hpp"

#include <string>
#include <numeric>
#include <algorithm>
#include <rlib/class_decorator.hpp>
#include "connection.hpp"
using std::string;

#ifndef WIN32
#include <arpa/inet.h>
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
        rlog.info("Socks5: Connection {} opened."_format(m_fd));
        try
        {
            handshake_pkgs();
            auto rawAddr = connect_pkgs();
            SnakeConnection nextHop(m_server.skserver);
            auto resolvedAddr = dns_pkgs(nextHop, rawAddr);
            passData(nextHop, resolvedAddr);
        }
        catch(std::exception &e)
        {
            rlog.error("Exception caught from child thread: At connection {}:{}."_format(m_fd, e.what()));
        }
        rlog.info("Socks5: Connection {} closed."_format(m_fd));
    }

    class addr_info
    {
    public:
        enum class addr_t {null, ipv4, ipv6, domain};
        addr_info(): type(addr_t::null) {};
        template<typename init_t>
        addr_info(init_t addr_initer, uint16_t port = 0) : port(port) {this->set(addr_initer);}

        void set(const std::string &domain) {
            type = addr_t::domain;
            data = std::string(domain);
        }
        void set(const std::array<uint8_t, 4> &v4addr) {
            type = addr_t::ipv4;
            data = std::string(std::begin(v4addr), std::end(v4addr));
        }
        void set(const std::array<uint8_t, 16> &v6addr) {
            type = addr_t::ipv6;
            data = std::string(std::begin(v6addr), std::end(v6addr));
        }
        void setport(uint16_t port) {this->port = port;} //host byte seq.
        auto getport() {return port;}

        bool operator!() const {
            auto sum = std::accumulate(data.cbegin(), data.cend(), (char)0);
            return sum == 0 || type == addr_t::null;
        }
        void try_6to4() {
            if(type != addr_t::ipv6)
                die("incorrect type.");
            bool conv_ok = true;
            std::for_each(data.cbegin() + 4, data.cend(), [&conv_ok](char c){if(c!='\0') conv_ok=false;});
            if(conv_ok)
            {
                data = data.substr(0, 4);
                type = addr_t::ipv4;
            }
        }

        client_query to_query_template() const
        {
            //port number must htons(), which's omitted in socks5.cc::unpackConnectionInfo()
            if(type == addr_t::null)
                die("convert null addr to query template.");
            if(type == addr_t::domain)
                die("convert addr to query template before resolve its domain.");
            if(port == 0)
                die("convert addr to query template before assign port number.");
            client_query cq { {0}, port };
            std::memcpy(cq.destination_ip, data.data(), (type == addr_t::ipv4 ? 4 : 16));
            return std::move(cq);
        }
        client_query make_dns_query_body() const
        {
            const auto &domain = data;
            binary_safe_string bs {true, 0, nullptr};
            bs.length = 32 + domain.size() + 1;
            bs.str = (char *)malloc(bs.length);

            // TODO: Remove this magic.
            std::memcpy(bs.str, "__m_str_dns_head_32__snakesocks_", 32);
            std::memcpy(bs.str + 32, domain.c_str(), domain.size());
            bs.str[bs.length - 1] = '\0';

            client_query cq { {0}, htons(port), bs };
            return std::move(cq);
        }
        std::string print() const
        {
            std::string res;
            switch(type)
            {
            case addr_t::null:
                res += "null";
                break;
            case addr_t::ipv4:
                res += "ipv4";
                goto dump;
            case addr_t::ipv6:
                res += "ipv6";
                goto dump;
            case addr_t::domain:
                res += "domain";
                res += ' ';
                res += data;
                break;
            dump:
                res += ' ';
                std::for_each(data.cbegin(), data.cend(), [&](const auto &e){res+=std::to_string((int)e)+'|';});
                break;
            }
            return std::move(res);
        }
        addr_t type;
    private:
        std::string data;
    private:
        uint16_t port;
    };

private:
    static auto unpackConnectionPacket(const char *pkgStr);
    void handshake_pkgs();
    addr_info connect_pkgs(); //return a client_query
    addr_info dns_pkgs(SnakeConnection &nextHop, addr_info &toResolve);
    
    void passData(SnakeConnection &nextHop, const addr_info &resolvedAddr); //This function is not abstracted, so this header is not designed as a library....

    fd m_fd;
    const Socks5Server &m_server;
};

#endif //SNAKESOCKS_SOCKS5_HPP
