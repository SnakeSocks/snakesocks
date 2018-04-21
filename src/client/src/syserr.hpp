#ifndef _SNAKESOCKS_SYSERR_HPP
#define _SNAKESOCKS_SYSERR_HPP 1

#include <rlib/string.hpp>
using namespace rlib::literals;

#include "debug.hpp"
#include <cstring>
#include <string>
#include <exception>
#include <stdexcept>

using std::printf;

#define _do_sys_except(msg) do{ \
                            ::std::string __errmsg = "sys error at {}:{} | errno={}:{} >"_format(__FILE__, __LINE__, errno, strerror(errno)); \
                            __errmsg += (msg); \
                            rlog.fatal(__errmsg); \
                            throw std::runtime_error(__errmsg); \
                        }while(0)
#define sysdie(msg) _do_sys_except(msg)

#define _do_except(msg) do{ \
                            ::std::string __errmsg = "logic/argument error at {}:{} | errno={}:{}(usually not help) >"_format(__FILE__, __LINE__, errno, strerror(errno)); \
                            __errmsg += (msg); \
                            rlog.fatal(__errmsg); \
                            throw std::runtime_error(__errmsg); \
                        }while(0)
#define die(msg) _do_except(msg)
#endif //_SNAKESOCKS_SYSERR_HPP
