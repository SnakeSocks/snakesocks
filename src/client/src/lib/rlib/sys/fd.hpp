#ifndef R_FDSET_HPP
#define R_FDSET_HPP

#include <unistd.h>
#include <sys/types.h>
#include <fstream>
#include <iostream>

#include <rlib/sys/os.hpp>
#if RLIB_OS_ID == OS_WINDOWS
using fd = HANDLE;
#else
using fd = int;
#endif

// Unfinished. I'm not sure if I must implement it.
#if 1+1 == 3
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
#endif
#endif

namespace rlib{
    class [[deprecated]] FileDescriptorSet
    {
    public:
        FileDescriptorSet() : m_size(0), maxFileDescriptor(0) {FD_ZERO(&m_fds_data);}
        void push(fd FileDescriptor) {FD_SET(FileDescriptor, &m_fds_data); ++m_size; maxFileDescriptor = (maxFileDescriptor > FileDescriptor ? maxFileDescriptor : FileDescriptor);}
        void pop(fd FileDescriptor) {FD_CLR(FileDescriptor, &m_fds_data); --m_size;} //It will break maxFileDescriptor.(for performance reason).
        void clear() {FD_ZERO(&m_fds_data); m_size = 0;maxFileDescriptor = 0;}
        bool check(fd FileDescriptor) {return FD_ISSET(FileDescriptor, &m_fds_data);}
        size_t size() const {return m_size;}
        int getMaxFileDescriptor() const {return maxFileDescriptor;}
        fd_set *getptr() {return &m_fds_data;}
    private:
        fd_set m_fds_data;
        size_t m_size;
        int maxFileDescriptor;
    };
}
#endif
