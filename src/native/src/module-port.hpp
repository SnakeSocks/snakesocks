#ifndef _SOCKSEX_MODULE_PORT_H
#define _SOCKSEX_MODULE_PORT_H 1

namespace port_c {
#include "../../modules/stdafx.h"
}

using port_c::binary_safe_string;
using port_c::connect_info;
using port_c::client_query;

/*
 * Note: Things to do while adding a new api:
 * module_interface::move_constructor()
 *              :: add new function pointer
 *              ::loadSo()
 *
 */


inline binary_safe_string make_binary_safe_string(uint32_t length) {
    binary_safe_string bss;
    bss.null_terminated = false;
    bss.length = length;
    bss.str = (char *)std::malloc(sizeof(char) * length);
    if(!bss.str)
        throw std::runtime_error("Unable to malloc bss.str.");

    return bss;
}
inline binary_safe_string make_binary_safe_string(const char *cstr, uint32_t length) {
    auto bss = make_binary_safe_string(length);
    std::memcpy(bss.str, cstr, length);
    return bss;
}
inline binary_safe_string make_binary_safe_string(std::string str) {
    auto bss = make_binary_safe_string(str.size());
    std::memcpy(bss.str, str.c_str(), str.size());
    return bss;
}
inline void delete_binary_safe_string(binary_safe_string &bss) {
    std::free(bss.str);
}

#endif //_SOCKSEX_MODULE_PORT_H
