#ifndef _SOCKSEX_MODULE_PORT_H
#define _SOCKSEX_MODULE_PORT_H 1

#include "../../modules/stdafx.h"

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
//    [[/*deprecated("Just used for type deduction"), */maybe_unused]] binary_safe_string client_make_auth_query(connect_info *);
//    [[/*deprecated("Just used for type deduction"), */maybe_unused]] _Bool client_deal_auth_reply(connect_info *, binary_safe_string server_reply); //Return true if auth is ok.
//    [[/*deprecated("Just used for type deduction"), */maybe_unused]] binary_safe_string client_encode(connect_info *,client_query payload); //I'll call it per complete packet.
//    [[/*deprecated("Just used for type deduction"), */maybe_unused]] client_query client_decode(connect_info *, binary_safe_string data); //I'll call it per complete packet.
//    [[/*deprecated("Just used for type deduction"), */maybe_unused]] void client_connection_close(connect_info *); //Be caution: this function may be called before client_deal_auth_reply().

    typedef std::add_pointer<decltype(client_make_auth_query)>::type _pf_client_make_auth_query_t;
    typedef std::add_pointer<decltype(client_deal_auth_reply)>::type _pf_client_deal_auth_reply_t;
    typedef std::add_pointer<decltype(client_encode)>::type _pf_client_encode_t;
    typedef std::add_pointer<decltype(client_decode)>::type _pf_client_decode_t;
    typedef std::add_pointer<decltype(client_connection_close)>::type _pf_client_connection_close_t;
}

#endif //_SOCKSEX_MODULE_PORT_H
