#include "NetLib.hpp"
#include <cstdio>
#include <cstring> //memset
#include <stdexcept>
/*
extern "C" {
int hostname_to_ip(const char *hostname,
                   char *ip)
{ // From http://www.binarytides.com/hostname-to-ip-address-c-sockets-linux/
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
    if (NULL == (he = gethostbyname(hostname)))
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }
    addr_list = (struct in_addr **) he->h_addr_list;
    for (i = 0; addr_list[i] != NULL; i++)
    {
        //Return the first one;
        strcpy(ip, inet_ntoa(*addr_list[i]));
        return 0;
    }
    return 1;
}
} // end extern C

string NetLib::resolveDomain4(const string &domain)
{
    if (isIpv4(domain)) return domain;
    if (isIpv6(domain)) throw std::runtime_error("ipv6 is currently unsupported.");
    char ipBuffer[64] = "";
    if(hostname_to_ip(domain.c_str(), ipBuffer) == 1) throw std::runtime_error("Failed to resolve domain " + domain + " to ipv4 address.");
    return std::move(string(ipBuffer));
}

#include <regex>

bool NetLib::isIpv4(const string &what)
{
    if (what.empty()) return false;
    std::regex ipFormat("^[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$");
    return std::regex_match(what, ipFormat);
}

#define _ipv6_template "^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))$"

bool NetLib::isIpv6(const string &what)
{
    if (what.empty()) return false;
    std::regex ipFormat(_ipv6_template);
    return std::regex_match(what, ipFormat);
}

bool NetLib::ipv4ToBytes(const string &ip, void *bytesBuf)
{
    in_addr tmp;
    if (inet_aton(ip.c_str(), &tmp) == 0)
        return false;
    memcpy(bytesBuf, &tmp.s_addr, 4);
    return true;
}

string NetLib::ipv4BytesToString(void *bytes)
{
    char *b = (char *)bytes;
    string toret;
    for(size_t cter = 0; cter < 4; ++cter)
    {
        toret += std::to_string((uint8_t)*(b + cter));
        toret += '.';
    }
    toret = toret.substr(0, toret.size() - 1);
    return std::move(toret);
}
*/
string NetLib::printData(void *bytes, size_t len)
{
    string buffer;
    for(size_t i=0;i<len;i++){
        char tmp[12];
        memset(tmp,0,12);
        sprintf(tmp,"%u ",(unsigned int)((unsigned char*)bytes)[i]);
        buffer+=tmp;
    }
    buffer+='\n';
    return std::move(buffer);
}


#include <boost/asio.hpp>

boost::asio::ip::address BoostNetLib::resolveDomainOnce(const string &domain) {
    return resolveDomainOnce(domain, [](const boost::asio::ip::address &)->bool {return true;});
}

boost::asio::ip::address BoostNetLib::resolveDomainOnce(const string &domain, std::function<bool(const boost::asio::ip::address &)> filter)
{
    // TODO: return all address and iterate them.
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(domain, "");
    for(boost::asio::ip::tcp::resolver::iterator i = resolver.resolve(query);
        i != boost::asio::ip::tcp::resolver::iterator();
        ++i)
    {
        boost::asio::ip::tcp::endpoint end = *i;
        if(filter(end.address()))
            return end.address();
    }
    throw std::runtime_error("BOOST Failed to resolve address " + domain);
}
