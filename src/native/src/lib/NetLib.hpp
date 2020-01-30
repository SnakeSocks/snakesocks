#ifndef GTKFILESHARE_NETLIB_HPP
#define GTKFILESHARE_NETLIB_HPP

#include <string>
#include <boost/asio.hpp>
using std::string;

//class /*[[deprecated]]*/ NetLib {
//public:
///*    static string resolveDomain4(const string &domain);
//    static bool isIpv4(const string &what);
//    static bool isIpv6(const string &what);
//    static bool ipv4ToBytes(const string &ip, void *bytesBuf);
//    static string ipv4BytesToString(void *bytes);
//*/    static string printData(void *bytes, size_t len);
//};

namespace BoostNetLib {
    boost::asio::ip::address resolveDomainOnce(const string &domain);
    boost::asio::ip::address resolveDomainOnce(const string &domain, std::function<bool(const boost::asio::ip::address &)> filter);
}

#include <rlib/sys/sio.hpp>
#include <rlib/string.hpp>
#include "module-port.hpp"
using rlib::literals::operator""_format;
#include "debug.hpp"

namespace NetLib {
    string printData(void *bytes, size_t len);
    inline void send_length_and_packet(sockfd_t connfd, void *data, uint32_t length) {
        if(length > 64*1024*1024)
            throw std::invalid_argument("send_length_and_packet: refuse to send too large packet: length=()"_format(length));
        uint32_t n = htonl(length);
        rlib::sockIO::sendn_ex(connfd, &n, sizeof(n));
        rlib::sockIO::sendn_ex(connfd, data, length);
    }
    // Warning: should free the return pointer.
    inline std::pair<void*, uint32_t> recv_length_and_packet(sockfd_t connfd) {
        uint32_t n;
        rlib::sockIO::recvn_ex(connfd, &n, sizeof(n));
        auto length = ntohl(n);
        if(length > 64*1024*1024)
            throw std::invalid_argument("recv_length_and_packet: refuse to recv too large packet: length=()"_format(length));
        void *data = malloc(length);
        rlib::sockIO::recvn_ex(connfd, data, length);
        //rlog.debug("recv.length+pkt, len={}, data={}", length, printData(data, length));
        return std::make_pair(data, length);
    }

    inline client_query assemble_dns_query(std::string domain) {
        binary_safe_string bs {true, 0, nullptr};
        bs.length = 32 + domain.size() + 1;
        bs.str = (char *)malloc(bs.length);

        std::memcpy(bs.str, "__m_str_dns_head_32__snakesocks_", 32);
        std::memcpy(bs.str + 32, domain.c_str(), domain.size());
        bs.str[bs.length - 1] = '\0';

        client_query cq { {0}, 0, bs };
        return cq;
    }
    inline bool is_dns_query(const client_query &cq) {
        // If destination_ip is zero, it's a dns package.
        for(const auto &ip_byte : cq.destination_ip) {
            if(ip_byte != 0)
                return false;
        }
        return true;
    }
    inline std::string get_domain_from_dns_query(const client_query &cq) {
        if(cq.payload.length <= 32)
            return ""; // not a dns query
        const char *domain = cq.payload.str + 32;
        return domain;
    }

    inline std::string ip_array_to_str(const uint8_t *_iparr) {
        const auto &iparr = *(const std::array<uint8_t, 16> *)_iparr;
        bool is_v6 = false;
        for(auto i = 4; i < 16; ++i) {
            if(iparr[i] != 0) {
                is_v6 = true;
                break;
            }
        }

        std::string result;
        for(auto cter = 0; cter < (is_v6 ? 16 : 4); ++cter) {
            result += std::to_string((uint32_t)iparr[cter]);
            result += is_v6 ? ':' : '.';
        }

        result.pop_back();
        return result;
    }
}


#endif //GTKFILESHARE_NETLIB_HPP
