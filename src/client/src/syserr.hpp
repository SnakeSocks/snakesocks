#ifndef _SNAKESOCKS_SYSERR_HPP
#define _SNAKESOCKS_SYSERR_HPP 1

#include <rlib/string/fstr.hpp>
using rlib::fstr;

#include <cstring>
#include <string>
#include <exception>
#include <stdexcept>

using std::printf;

#define _do_sys_except(msg, ...) do{ \
                            ::std::string __errmsg = fstr("sys error at %s:%d | errno=%d:%s >", __FILE__, __LINE__, errno, strerror(errno)); \
                            __errmsg += fstr(msg, ##__VA_ARGS__); \
                            __errmsg += "\n"; \
                            throw std::runtime_error(__errmsg); \
                        }while(0)
#define sysdie(msg, ...) _do_sys_except(msg, ##__VA_ARGS__)

#define _do_except(msg, ...) do{ \
                            ::std::string __errmsg = fstr("logic/argument error at %s:%d | errno=%d:%s(usually not help) >", __FILE__, __LINE__, errno, strerror(errno)); \
                            __errmsg += fstr(msg, ##__VA_ARGS__); \
                            __errmsg += "\n"; \
                            throw std::runtime_error(__errmsg); \
                        }while(0)
#define die(msg, ...) _do_except(msg, ##__VA_ARGS__)
#endif //_SNAKESOCKS_SYSERR_HPP
