#ifndef _SNAKESOCKS_SYSERR_HPP
#define _SNAKESOCKS_SYSERR_HPP 1

#include <rlib/string/string.hpp>

#include <cstring>
#include <string>
#include <exception>
#include <stdexcept>

using std::printf;

#define _do_sys_except(msg, ...) do{ \
                            ::std::string __errmsg = rlib::format_string("sys error at {}:{} | errno={}:{} >", __FILE__, __LINE__, errno, strerror(errno)); \
                            __errmsg += rlib::format_string(msg, ##__VA_ARGS__); \
                            __errmsg += "\n"; \
                            throw std::runtime_error(__errmsg); \
                        }while(0)
#define sysdie(msg, ...) _do_sys_except(msg, ##__VA_ARGS__)

#define _do_except(msg, ...) do{ \
                            ::std::string __errmsg = rlib::format_string("logic/argument error at {}:{} | errno={}:{}(usually not help) >", __FILE__, __LINE__, errno, strerror(errno)); \
                            __errmsg += rlib::format_string(msg, ##__VA_ARGS__); \
                            __errmsg += "\n"; \
                            throw std::runtime_error(__errmsg); \
                        }while(0)
#define die(msg, ...) _do_except(msg, ##__VA_ARGS__)
#endif //_SNAKESOCKS_SYSERR_HPP
