#ifndef SNAKESOCKSCLI_FIX_CONFIG_HPP
#define SNAKESOCKSCLI_FIX_CONFIG_HPP

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <string>
#include <exception>

namespace pt = boost::property_tree;
using std::string;

class client_config
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
        logFile = tree.get("core.logFile", std::string(""));
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

class server_config {
public:
    string bindIp;
    uint16_t bindPort;
    string passphrase;

    string modulePath;
    int debugLevel;
    string logFile;
    //http://www.boost.org/doc/libs/1_64_0/libs/property_tree/examples/debug_settings.cpp
    void load(const string &confPath)
    {
        pt::ptree tree;
        pt::read_ini(confPath, tree);
        bindIp = tree.get<string>("core.ip", "::0");
        bindPort = tree.get<uint16_t>("core.port");
        passphrase = tree.get<string>("core.passphrase");
        modulePath = tree.get<string>("core.module");
        debugLevel = tree.get("extra.debugLevel", 1);
        logFile = tree.get("extra.logFile", std::string(""));
    }
    void save(const string &confPath)
    {
        pt::ptree tree;
        tree.put("core.ip", bindIp);
        tree.put("core.port", bindPort);
        tree.put("core.passphrase", passphrase);
        tree.put("core.module", modulePath);
        tree.put("extra.debugLevel", debugLevel);
        tree.put("extra.logFile", logFile);
        pt::write_ini(confPath, tree);
    }
};


#endif //SNAKESOCKSCLI_FIX_CONFIG_HPP
