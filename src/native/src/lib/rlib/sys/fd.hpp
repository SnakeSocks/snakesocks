#ifndef RLIB_FD_HPP_
#define RLIB_FD_HPP_

#include <rlib/sys/os.hpp>
#if RLIB_OS_ID == OS_WINDOWS
#include <Windows.h>
using fd_t = HANDLE;
using sockfd_t = SOCKET;
#else
using fd_t = int;
using sockfd_t = int;
#endif

#endif
