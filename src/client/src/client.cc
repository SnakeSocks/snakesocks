#include "socks5.hpp"
#include "debug.hpp"
#include "config.hpp"
#include "syserr.hpp"

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
int _runtime_debugLevel;

using namespace std;

int main_proc(config &conf)
{
    _runtime_debugLevel = conf.debugLevel;
    cout << "Setting debug level = " << _runtime_debugLevel << endl;
    client_module mod(conf.modulePath);
    s5server sss(conf.bindIp, conf.bindPort, tunnel(conf.serverIp, conf.serverPort, conf.passphrase, std::move(mod)));
    sss.run();
    return 0;
}

void display_usage()
{
#define n "\n"
    printf(
#ifdef FOR_MINGW_WIN
            "SnakeSocks Client 1.2.0 Windows Edition" n n
#else
            "SnakeSocks Client 1.2.0" n n
#endif
            "Usage: snakesockscli [OPTION...]" n
            "A extensible proxy which supports module-defined protocol behavior." n
            "You can install and run module easily from configuration file," n
            "to determine how packets are encrypted, transmitted, etc." n n
            "Options:" n
            "-s <server address>\tServer ip or domain" n
            "-p <server port>\tServer listening port" n
            "-k <passphrase>\tPassphrase" n
            "-L <listen address>\tLocal address that socks5 server listens on" n
            "-P <listen port>\tLocal port that socks5 server listens on" n
            "-D <debug level>\tDebug level to set" n
            "-m <path to module>\tModule path" n
            n
            "-c <config file path>\tPath to config file(default: /etc/snakesocks/conf/client.conf)" n
            "                     \t    Use '-c NULL' to prevent me from looking for client.conf" n
            "-d\tRun client as daemon" n
            "-l <log file name>\tLog file for daemon mode(default: /var/log/skcli.log)" n
            "-h\tShow this message" n n
            "Published on GNU license V2." n
    );
#undef n
}

int ____main(int arglen, char **argv)
{
    int dflag = 0;
    char *confPath = NULL;
    char *daemonLogFilePath = NULL;
    char *serverAddr = NULL, *serverPort = NULL, *passP = NULL, *listenAddr = NULL, *listenPort = NULL;
    char *debugLev = NULL, *modPath = NULL;
    int index;
    int c;

    opterr = 0;

    while ((c = getopt (arglen, argv, "c:dhl:s:p:k:L:P:D:m:")) != -1)
        switch (c)
        {
            case 'd':
                dflag = 1;
                break;
            case 'c':
                confPath = optarg;
                break;
            case 'h':
                display_usage();
                exit(0);
                break;
            case 'l':
                daemonLogFilePath = optarg;
                break;
            case 's':
                serverAddr = optarg;
                break;
            case 'p':
                serverPort = optarg;
                break;
            case 'k':
                passP = optarg;
                break;
            case 'L':
                listenAddr = optarg;
                break;
            case 'P':
                listenPort = optarg;
                break;
            case 'D':
                debugLev = optarg;
                break;
            case 'm':
                modPath = optarg;
                break;
            case '?':
                if (optopt == 'c')
                    printf("Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    printf("Unknown option `-%c'.\n", optopt);
                else
                    printf("Unknown option character `\\x%x'.\n", optopt);
                display_usage();
                return 1;
            default:
                abort();
        }

    config conf;
    if(confPath && (strcmp(confPath, "NULL") == 0))
    { // NO configuration file.
        printf("No config mode.\n");
        conf.daemonLogFile = daemonLogFilePath ? daemonLogFilePath : "/var/log/skcli.log"; //Not required.

        if(!serverAddr) die("servAddr not set."); conf.serverIp = serverAddr;
        if(!serverPort) die("servPort not set."); conf.serverPort = atoi(serverPort);
        if(!passP) die("passphrase not set."); conf.passphrase = passP;
        if(!listenAddr) die("bindAddr not set."); conf.bindIp = listenAddr;
        if(!listenPort) die("bindPort not set."); conf.bindPort = atoi(listenPort);
        if(!debugLev) die("debugLevel not set."); conf.debugLevel = atoi(debugLev);
        if(!modPath) die("modulePath not set."); conf.modulePath = modPath;
    }
    else
    {
        conf.load(confPath == NULL ? "/etc/snakesocks/conf/client.conf" : confPath);
        if(daemonLogFilePath) conf.daemonLogFile = daemonLogFilePath;
        if(serverAddr) conf.serverIp = serverAddr;
        if(serverPort) conf.serverPort = atol(serverPort);
        if(passP) conf.passphrase = passP;
        if(listenAddr) conf.bindIp = listenAddr;
        if(listenPort) conf.bindPort = atol(listenPort);
        if(debugLev) conf.debugLevel = atoi(debugLev);
        if(modPath) conf.modulePath = modPath;
    }

    if(dflag)
    {
#ifdef WIN32 
        die("Daemon mode is not supported on Windows.");
#else
        const char *logFile = conf.daemonLogFile.c_str();

        //redirect stdout/stderr to file
        int out = open(logFile, O_RDWR|O_CREAT|O_APPEND, 0600);
        if (-1 == out) { sysdie("Failed to open log file %s", logFile); }

        int err = open(logFile, O_RDWR|O_CREAT|O_APPEND, 0600);
        if (-1 == err) { sysdie("Failed to open log file %s", logFile); }

        int save_out = dup(fileno(stdout));
        int save_err = dup(fileno(stderr));

        if (-1 == dup2(out, fileno(stdout))) { sysdie("Failed to redirect stdout"); }
        if (-1 == dup2(err, fileno(stderr))) { sysdie("Failed to redirect stderr"); }

        if(daemon(0, 1) == -1) sysdie("Failed to launch daemon.");
#endif
    }

    return main_proc(conf);
}

int main(int arglen, char **argv)
{
#ifdef WIN32
	//WIN32 cannot deal with exception elegently. catch them!
	try {____main(arglen, argv);}
	catch (std::exception &e) 
	{
		cout << "Uncaught Exception: " << ' ' << e.what() << endl;
		return 2;
	}
	return 0;
#else
	return ____main(arglen, argv);
#endif
}
