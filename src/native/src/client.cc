#include "socks5.hpp"
#include "debug.hpp"
#include "config.hpp"
#include "syserr.hpp"

#include <rlib/opt.hpp>
#include <rlib/stdio.hpp>
#include <rlib/macro.hpp>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

rlib::logger rlog(std::cout);

using namespace std;
using rlib::println;
using rlib::print;

int skcli_main(client_config &&conf)
{
    switch(conf.debugLevel) {
        case 0:
            rlog.set_log_level(rlib::log_level_t::FATAL);
            break;
        case 1:
            rlog.set_log_level(rlib::log_level_t::ERROR);
            break;
        case 2:
            rlog.set_log_level(rlib::log_level_t::INFO);
            break;
        case 3:
        case 4: // super_debug mode is removed.
            rlog.set_log_level(rlib::log_level_t::DEBUG);
            break;
        default:
            die("Unknown log_level {}."_format(conf.debugLevel));
    }
    module_interface mod(conf.modulePath);
    Socks5Server sss(conf.bindIp, conf.bindPort, outbound_tunnel(conf.serverIp, conf.serverPort, conf.passphrase, std::move(mod), conf.retryResolve));
    sss.listen();
    return 0;
}

void display_usage()
{
#define n "\n"
    print(
            "SnakeSocks client"
#ifdef SK_VERSION
            " " RLIB_MACRO_TO_CSTR(SK_VERSION)
#endif
#if RLIB_OS_ID == OS_WINDOWS
            " Windows Edition"
#endif
            n

#ifdef COMPILE_TIME
#define COMPILE_TIME_STR RLIB_MACRO_TO_CSTR(COMPILE_TIME)
            "Compiled at UTC " COMPILE_TIME_STR n
#endif

            n
            "Usage: skcli [OPTION...]" n
            "A extensible proxy which supports module-defined protocol behavior." n
            "You can install and run module easily from configuration file," n
            "to determine how packets are encrypted, transmitted, etc." n n
            "Options:" n
            "-s --server <server address>    Server ip or domain" n
            "-p --server-port <server port>  Server listening port" n
            "-k --passphrase <passphrase>    Passphrase" n
            "-L --listen <listen address>    Local address that socks5 server listens on" n
            "-P --listen-port <listen port>  Local port that socks5 server listens on" n
            "-D --debug <debug level>        Debug level to set (0/1/2/3/4)" n
            "-m --mod <path to module>       Module path" n
            "   --retry                      Retry infinitely if a domain cannot be resolved"
            n
#if RLIB_OS_ID == OS_WINDOWS
            "-c --conf <config file path>    Path to config file(default: conf\\client.conf)" n
            "                                    Use '-c NULL' to prevent me looking for client.conf" n
#else
            "-c --conf <config file path>    Path to config file (default: /etc/snakesocks/conf/client.conf)" n
            "                                    Use '-c NULL' to prevent me looking for client.conf" n
            "-d --daemon                     Run client as daemon" n
            "-l --log <log file name>        Log file for daemon mode, use stdout if not set" n
#endif
            "-h --help                       Show this message" n n
            "Published on GNU license V2." n
    );
#undef n
}

int wrapped_main(int arglen, char **argv)
{
    rlib::opt_parser args(arglen, argv);

    if(args.getBoolArg("--help", "-h"))
    {
        display_usage();
        return 0;
    }

    bool asDaemon = args.getBoolArg("--daemon", "-d");
    bool retryResolve = args.getBoolArg("--retry");
    string confPath = args.getValueArg("--conf", "-c", false);
    string logFilePath = args.getValueArg("--log", "-l", false);
    string serverAddr = args.getValueArg("--server", "-s", false);
    string serverPort = args.getValueArg("--server-port", "-p", false);
    string passP = args.getValueArg("--passphrase", "-k", false);
    string listenAddr = args.getValueArg("--listen", "-L", false);
    string listenPort = args.getValueArg("--listen-port", "-P", false);
    string debugLev = args.getValueArg("--debug", "-D", false);
    string modPath = args.getValueArg("--mod", "-m", false);

    if(!args.allArgDone())
        rlog.warning("Some ill formed arguments are ignored.");

    client_config conf;
    if(confPath == "NULL")
    { // NO configuration file.
        rlog.info("No config mode.");
        conf.logFile = logFilePath;

        if(serverAddr.empty()) die("servAddr not set."); conf.serverIp = serverAddr;
        if(serverPort.empty()) die("servPort not set."); conf.serverPort = (uint16_t)stoi(serverPort);
        if(passP.empty()) die("passphrase not set."); conf.passphrase = passP;
        if(listenAddr.empty()) die("bindAddr not set."); conf.bindIp = listenAddr;
        if(listenPort.empty()) die("bindPort not set."); conf.bindPort = (uint16_t)stoi(listenPort);
        if(debugLev.empty()) die("debugLevel not set."); conf.debugLevel = stoi(debugLev);
        if(modPath.empty()) die("modulePath not set."); conf.modulePath = modPath;
    }
    else
    {
#if RLIB_OS_ID == OS_WINDOWS
        conf.load(confPath.empty() ? std::string("conf\\client.conf") : confPath);
#else
        conf.load(confPath.empty() ? std::string("/etc/snakesocks/conf/client.conf") : confPath);
#endif
        if(!logFilePath.empty()) conf.logFile = logFilePath;
        if(!serverAddr.empty()) conf.serverIp = serverAddr;
        if(!serverPort.empty()) conf.serverPort = (uint16_t)stoi(serverPort);
        if(!passP.empty()) conf.passphrase = passP;
        if(!listenAddr.empty()) conf.bindIp = listenAddr;
        if(!listenPort.empty()) conf.bindPort = (uint16_t)stoi(listenPort);
        if(!debugLev.empty()) conf.debugLevel = stoi(debugLev);
        if(!modPath.empty()) conf.modulePath = modPath;
    }

    if(retryResolve) conf.retryResolve = true;

    if(asDaemon)
    {
#if RLIB_OS_ID == OS_WINDOWS 
        die("Daemon mode is not supported on Windows.");
#else
        if(daemon(0, 1) == -1) sysdie("Failed to launch daemon.");
#endif
    }
    if(!conf.logFile.empty()) {
        rlog.info("Log redirected to {}."_format(conf.logFile));
        rlog = rlib::logger(conf.logFile);
    }

    return skcli_main(std::move(conf));
}

int main(int arglen, char **argv)
{
    rlog.set_log_level(rlib::log_level_t::DEBUG);
	// WIN32 cannot deal with exception elegently. catch them!
    // Grap exception and print `fatal` log is also useful on linux
    //     , so I'll apply this wrap for every os.
	try { wrapped_main(arglen, argv); }
	catch (std::exception &e) 
	{
        rlog.fatal("Uncaught Exception: {}"_format(e.what()));
		return 2;
	}
	return 0;
}
