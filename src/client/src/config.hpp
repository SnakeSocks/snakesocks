#ifndef SNAKESOCKSCLI_FIX_CONFIG_HPP
#define SNAKESOCKSCLI_FIX_CONFIG_HPP

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <string>
#include <exception>
#include "NetLib.hpp"
namespace pt = boost::property_tree;
using std::string;

class config
{
public:
    string serverIp;
    uint16_t serverPort;
    string passphrase;

    string bindIp;
    uint16_t bindPort;

    string modulePath;
    int debugLevel;
    string daemonLogFile;
    //http://www.boost.org/doc/libs/1_64_0/libs/property_tree/examples/debug_settings.cpp
    void load(const string &confPath)
    {
        pt::ptree tree;
        pt::read_ini(confPath, tree);
        serverIp = tree.get<string>("server.ip");
        serverPort = tree.get<uint16_t>("server.port");
        passphrase = tree.get<string>("server.passphrase");
        bindIp = tree.get<string>("socks5.ip");
        bindPort = tree.get<uint16_t>("socks5.port");
        modulePath = tree.get<string>("core.module");
        debugLevel = tree.get("core.debugLevel", 1);
        daemonLogFile = tree.get("core.daemonLogFile", std::string("/var/log/skcli.log"));
    }
    void save(const string &confPath)
    {
        pt::ptree tree;
        tree.put("server.ip", serverIp);
        tree.put("server.port", serverPort);
        tree.put("server.passphrase", passphrase);
        tree.put("socks5.ip", bindIp);
        tree.put("socks5.port", bindPort);
        tree.put("core.module", modulePath);
        tree.put("core.debugLevel", debugLevel);
        tree.put("core.daemonLogFile", daemonLogFile);
        pt::write_ini(confPath, tree);
    }
};


#endif //SNAKESOCKSCLI_FIX_CONFIG_HPP
