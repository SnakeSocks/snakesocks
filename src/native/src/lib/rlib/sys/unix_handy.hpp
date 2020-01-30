#ifndef RLIB_UNIX_HANDY_HPP_
#define RLIB_UNIX_HANDY_HPP_

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <rlib/scope_guard.hpp>
#include <rlib/string.hpp>

#include <rlib/sys/os.hpp>
#if RLIB_OS_ID == OS_WINDOWS
#error rlib/sys/unix_handy.hpp is not for Windows.
#endif

// Deprecated. Use sys/sio.hpp
#if 1+1 == 4
namespace rlib {
    namespace impl {
        using rlib::literals::operator""_format;
        static inline fd unix_quick_listen(const std::string &addr, uint16_t port, bool use_udp = false) {
            addrinfo *psaddr;
            addrinfo hints{0};
            fd listenfd;

            hints.ai_family = AF_UNSPEC;
            if(use_udp) {
                hints.ai_socktype = SOCK_DGRAM;
                hints.ai_protocol = IPPROTO_UDP;
            }
            else {
                hints.ai_socktype = SOCK_STREAM;
                hints.ai_protocol = IPPROTO_TCP;
            }
            hints.ai_flags = AI_PASSIVE;    /* For listen */
            auto _ = getaddrinfo(addr.c_str(), std::to_string(port).c_str(), &hints, &psaddr);
            if (_ != 0) throw std::runtime_error("Failed to getaddrinfo. returnval={}, check `man getaddrinfo`'s return value."_format(_));

            bool success = false;
            for (addrinfo *rp = psaddr; rp != nullptr; rp = rp->ai_next) {
                listenfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
                if (listenfd == -1)
                    continue;
                int reuse = 1;
                if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse, sizeof(int)) < 0)
                    throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
                if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, (const char *) &reuse, sizeof(int)) < 0)
                    throw std::runtime_error("setsockopt(SO_REUSEPORT) failed");
                if (bind(listenfd, rp->ai_addr, rp->ai_addrlen) == 0) {
                    success = true;
                    break;
                }
                close(listenfd);
            }
            if (!success) throw std::runtime_error("Failed to bind {}:{}."_format(addr, port));

            if (-1 == ::listen(listenfd, 16)) throw std::runtime_error("listen failed.");

            rlib_defer([psaddr] { freeaddrinfo(psaddr); });
            return listenfd;
        }

        static inline fd unix_quick_connect(const std::string &addr, uint16_t port, bool use_udp = false) {
            addrinfo *paddr;
            addrinfo hints{0};
            fd sockfd;

            hints.ai_family = AF_UNSPEC;
            if(use_udp) {
                hints.ai_socktype = SOCK_DGRAM;
                hints.ai_protocol = IPPROTO_UDP;
            }
            else {
                hints.ai_socktype = SOCK_STREAM;
                hints.ai_protocol = IPPROTO_TCP;
            }
            auto _ = getaddrinfo(addr.c_str(), std::to_string(port).c_str(), &hints, &paddr);
            if (_ != 0)
                throw std::runtime_error("getaddrinfo failed. Check network connection to {}:{}; returnval={}, check `man getaddrinfo`'s return value."_format(
                        addr.c_str(), port, _));
            rlib_defer([paddr] { freeaddrinfo(paddr); });

            bool success = false;
            for (addrinfo *rp = paddr; rp != NULL; rp = rp->ai_next) {
                sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
                if (sockfd == -1)
                    continue;
                int reuse = 1;
                if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse, sizeof(int)) < 0)
                    throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
                if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char *) &reuse, sizeof(int)) < 0)
                    throw std::runtime_error("setsockopt(SO_REUSEPORT) failed");
                if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
                    success = true;
                    break; /* Success */
                }
                close(sockfd);
            }
            if (!success) throw std::runtime_error("Failed to connect to any of these addr.");

            return sockfd;
        }
    }
    using impl::unix_quick_connect;
    using impl::unix_quick_listen;
}
#endif


// Unfinished. I'm not sure if I must implement it.
#if 1+1 == 3
#include <fstream>
#include <iostream>
#if RLIB_COMPILER_ID == CC_GCC 
#include <ext/stdio_filebuf.h>
namespace rlib {
    inline std::istream & fd_to_istream(fd handle) {
        if(handle == STDIN_FILENO) return std::cin;
        __gnu_cxx::stdio_filebuf<char> filebuf(handle, std::ios::in);
        return std::istream(&filebuf);
    }
    inline std::ostream & fd_to_ostream(fd handle) {
        if(handle == STDOUT_FILENO) return std::cout;
        if(handle == STDERR_FILENO) return std::cerr;
        __gnu_cxx::stdio_filebuf<char> filebuf(handle, std::ios::out);
        return std::ostream(&filebuf);
    }
    inline std::iostream & fd_to_iostream(fd handle) {
        __gnu_cxx::stdio_filebuf<char> filebuf(handle, std::ios::in | std::ios::out);
        return std::iostream(&filebuf);
    }
} // rlib
#elif RLIB_COMPILER_ID == CC_MSVC
namespace rlib {
    inline std::istream & fd_to_istream(fd handle) {
        if(handle == STDIN_FILENO) return std::cin;
        ifstream fs(::_fdopen(handle, "r"));
        return fs;
    }
    inline std::ostream & fd_to_ostream(fd handle) {
        if(handle == STDOUT_FILENO) return std::cout;
        if(handle == STDERR_FILENO) return std::cerr;
        ofstream fs(::_fdopen(handle, "w"));
        return fs;
    }
    inline std::iostream & fd_to_iostream(fd handle) {
        fstream fs(::_fdopen(handle, "rw"));
        return fs;
    }
} // rlib
#else
namespace rlib {
    constexpr inline std::istream & fd_to_istream(fd handle) {
        if(handle == STDIN_FILENO) return std::cin;
        throw std::invalid_argument("fd != 0 to istream is not implemented except gcc/msvc.");
    }
    constexpr inline std::ostream & fd_to_ostream(fd handle) {
        if(handle == STDOUT_FILENO) return std::cout;
        if(handle == STDERR_FILENO) return std::cerr;
        throw std::invalid_argument("fd != 1/2 to ostream is not implemented except gcc/msvc.");
    }
    constexpr inline std::iostream & fd_to_iostream(fd handle) {
        throw std::invalid_argument("fd to iostream is not implemented except gcc/msvc.");
    }

} // rlib
#endif // if compiler
#endif // if 1+1==3

#endif // header guarder
