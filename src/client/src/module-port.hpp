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
 * client_module::move_constructor()
 *              :: add new function pointer
 *              ::loadSo()
 *
 * ____trick:: add new typedef
 */

namespace ____trick
{
    typedef std::add_pointer<decltype(port_c::client_make_auth_query)>::type _pf_client_make_auth_query_t;
    typedef std::add_pointer<decltype(port_c::client_deal_auth_reply)>::type _pf_client_deal_auth_reply_t;
    typedef std::add_pointer<decltype(port_c::client_encode)>::type _pf_client_encode_t;
    typedef std::add_pointer<decltype(port_c::client_decode)>::type _pf_client_decode_t;
    typedef std::add_pointer<decltype(port_c::client_connection_close)>::type _pf_client_connection_close_t;
}

#endif //_SOCKSEX_MODULE_PORT_H
