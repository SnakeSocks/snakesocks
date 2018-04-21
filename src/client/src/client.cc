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

int main_proc(config &&conf)
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
    client_module mod(conf.modulePath);
    Socks5Server sss(conf.bindIp, conf.bindPort, tunnel(conf.serverIp, conf.serverPort, conf.passphrase, std::move(mod), conf.retryResolve));
    sss.listen();
    return 0;
}

void display_usage()
{
#define n "\n"
    print(
            "SnakeSocks client"
#ifdef SKCLI_VERSION
            " " RLIB_MACRO_TO_CSTR(SKCLI_VERSION)
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
            "Usage: snakesockscli [OPTION...]" n
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
            "                                    Use '-c NULL' to prevent me from looking for client.conf" n
#else
            "-c --conf <config file path>    Path to config file(default: /etc/snakesocks/conf/client.conf)" n
            "                                    Use '-c NULL' to prevent me from looking for client.conf" n
            "-d --daemon                     Run client as daemon" n
            "-l --daemon-log <log file name> Log file for daemon mode(default: /var/log/skcli.log)" n
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
    string confPath = args.getValueArg("--conf", "-c");
    string daemonLogFilePath = args.getValueArg("--daemon-log", "-l");
    string serverAddr = args.getValueArg("--server", "-s");
    string serverPort = args.getValueArg("--server-port", "-p");
    string passP = args.getValueArg("--passphrase", "-k");
    string listenAddr = args.getValueArg("--listen", "-L");
    string listenPort = args.getValueArg("--listen-port", "-P");
    string debugLev = args.getValueArg("--debug", "-D");
    string modPath = args.getValueArg("--mod", "-m");

    if(!args.allArgDone())
        println("Warning: some ill formed arguments are ignored.");

    config conf;
    if(confPath == "NULL")
    { // NO configuration file.
        println("No config mode.");
        conf.daemonLogFile = daemonLogFilePath.empty() ? daemonLogFilePath : std::string("/var/log/skcli.log"); //Not required.

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
        if(!daemonLogFilePath.empty()) conf.daemonLogFile = daemonLogFilePath;
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
        rlog = rlib::logger(conf.daemonLogFile);
//        const char *logFile = conf.daemonLogFile.c_str();

        //redirect stdout/stderr to file
//        int out = open(logFile, O_RDWR|O_CREAT|O_APPEND, 0600);
//        if (-1 == out) { sysdie("Failed to open log file"s + logFile); }
//
//        int err = open(logFile, O_RDWR|O_CREAT|O_APPEND, 0600);
//        if (-1 == err) { sysdie("Failed to open log file"s + logFile); }
//
//        int save_out = dup(fileno(stdout));
//        int save_err = dup(fileno(stderr));
//
//        if (-1 == dup2(out, fileno(stdout))) { sysdie("Failed to redirect stdout"); }
//        if (-1 == dup2(err, fileno(stderr))) { sysdie("Failed to redirect stderr"); }

        if(daemon(0, 1) == -1) sysdie("Failed to launch daemon.");
#endif
    }

    return main_proc(std::move(conf));
}

int main(int arglen, char **argv)
{
    rlog.set_log_level(rlib::log_level_t::DEBUG);
#if RLIB_OS_ID == OS_WINDOWS
	//WIN32 cannot deal with exception elegently. catch them!
	try { wrapped_main(arglen, argv); }
	catch (std::exception &e) 
	{
        println("Uncaught Exception: ", e.what());
		return 2;
	}
	return 0;
#else
	return wrapped_main(arglen, argv);
#endif
}
