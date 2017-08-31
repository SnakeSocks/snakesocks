#ifndef GTKFILESHARE_NETLIB_HPP
#define GTKFILESHARE_NETLIB_HPP

#include <string>
#include <boost/asio.hpp>
using std::string;

class /*[[deprecated]]*/ NetLib {
public:
/*    static string resolveDomain4(const string &domain);
    static bool isIpv4(const string &what);
    static bool isIpv6(const string &what);
    static bool ipv4ToBytes(const string &ip, void *bytesBuf);
    static string ipv4BytesToString(void *bytes);
*/    static string printData(void *bytes, size_t len);
};

class BoostNetLib {
public:
    static boost::asio::ip::address resolveDomainOnce(const string &domain);
    static boost::asio::ip::address resolveDomainOnce(const string &domain, std::function<bool(const boost::asio::ip::address &)> filter);
};


#endif //GTKFILESHARE_NETLIB_HPP
