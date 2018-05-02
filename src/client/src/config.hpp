#ifndef SNAKESOCKSCLI_FIX_CONFIG_HPP
#define SNAKESOCKSCLI_FIX_CONFIG_HPP

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <string>
#include <exception>

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
    bool retryResolve;
    int debugLevel;
    string logFile;
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
        retryResolve = tree.get<string>("core.nx_retry") == "true";
        debugLevel = tree.get("core.debugLevel", 1);
        logFile = tree.get("core.logFile", std::string("stdout"));
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
        tree.put("core.nx_retry", retryResolve ? "true" : "false");
        tree.put("core.debugLevel", debugLevel);
        tree.put("core.logFile", logFile);
        pt::write_ini(confPath, tree);
    }
};


#endif //SNAKESOCKSCLI_FIX_CONFIG_HPP
