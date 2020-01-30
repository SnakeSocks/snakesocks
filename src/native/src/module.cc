#include <cstring>
#include "module.hpp"
#include "syserr.hpp"
#include "NetLib.hpp"
#include <rlib/macro.hpp>

binary_safe_string module_interface::client_impl::makeAuthQuery(connect_info *conn) const
{
    return _f_client_make_auth_query(conn);
}

bool module_interface::client_impl::onAuthReply(char *authdata, size_t len, connect_info *conn) const
{
    binary_safe_string server_reply;
    server_reply.null_terminated = false;
    server_reply.length = static_cast<uint32_t>(len);
    server_reply.str = (char *)authdata;

    return _f_client_deal_auth_reply(conn, server_reply);
}

binary_safe_string module_interface::client_impl::encode(binary_safe_string payload, connect_info *conn, client_query addrTemplate) const
{ //payload.str will be finally freed, and addrTemplate is passed by value.
    if(payload.length)
        addrTemplate.payload = payload;

    return _f_client_encode(conn, addrTemplate);
}

client_query module_interface::client_impl::decode(binary_safe_string payload, connect_info *conn) const
{
    return _f_client_decode(conn, payload);
}

void module_interface::client_impl::onCloseConn(connect_info *conn) const {
    _f_client_connection_close(conn);
}

binary_safe_string module_interface::server_impl::makeAuthReply(connect_info *conn, binary_safe_string query, bool *output_auth_failed) const {
    return _f_server_make_auth_reply(conn, query, output_auth_failed);
}

binary_safe_string module_interface::server_impl::encode(connect_info *conn, client_query payload) const {
    return _f_server_encode(conn, payload);
}

client_query module_interface::server_impl::decode(connect_info *conn, binary_safe_string payload) const {
    return _f_server_decode(conn, payload);
}

void module_interface::server_impl::onCloseConn(connect_info *conn) const {
    _f_server_connection_close(conn);
}


#if RLIB_OS_ID != OS_WINDOWS
//POSIX shared object
#include "module-port.hpp"
void module_interface::loadSo(const string &filePath)
{
    // using namespace ____trick;
    void *handle = dlopen(filePath.c_str(), RTLD_LAZY);
    char *errstr = NULL;
    if(!handle) sysdie("dlopen failed to open shared object {}, dlerror={}"_format(filePath.c_str(), dlerror()));
    dlerror();

#define RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(client_or_server, funcname) { \
        auto _pf_tmp = (std::add_pointer<decltype(port_c::funcname)>::type) dlsym(handle, RLIB_MACRO_TO_CSTR(funcname)); \
        if (_pf_tmp == NULL) sysdie("Unable to locate symbol {} in dylib {}. dlerror={}"_format(RLIB_MACRO_TO_CSTR(funcname), filePath.c_str(), dlerror())); \
        client_or_server.RLIB_MACRO_CAT(_f_, funcname) = _pf_tmp; \
    }

    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(client, client_make_auth_query)
    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(client, client_deal_auth_reply)
    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(client, client_encode)
    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(client, client_decode)
    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(client, client_connection_close)

    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(server, server_make_auth_reply)
    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(server, server_encode)
    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(server, server_decode)
    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(server, server_connection_close)

    so_handle = handle; //Here is master thread. sysdie will exit program and linux will close this handle on error automatically.
                     //No need for scope guard.
    return;
}

#else
//Win32 dll
void module_interface::loadSo(const string &filePath)
{
    using namespace ____trick;
    HMODULE handle = LoadLibrary(filePath.c_str());
    if(!handle) sysdie("Failed to open dll dylib {}, LastError={}. Confirm if it exists and you have access to it."_format(filePath.c_str(), GetLastError()));

#define RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(client_or_server, funcname) { \
        auto _pf_tmp = (std::add_pointer<decltype(port_c::funcname)>::type) GetProcAddress(handle, RLIB_MACRO_TO_CSTR(funcname)); \
        if (_pf_tmp == NULL) sysdie("Unable to locate symbol {} in dylib {}. LastError={}"_format(RLIB_MACRO_TO_CSTR(funcname), filePath.c_str(), GetLastError())); \
        client_or_server.RLIB_MACRO_CAT(_f_, funcname) = _pf_tmp; \
    }

    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(client, client_make_auth_query)
    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(client, client_deal_auth_reply)
    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(client, client_encode)
    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(client, client_decode)
    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(client, client_connection_close)

    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(server, server_make_auth_reply)
    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(server, server_encode)
    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(server, server_decode)
    RLIB_IMPL_LOAD_DYLIB_SYMBOL_HELPER(server, server_connection_close)
/*
    auto _pf_client_make_auth_query = (_pf_client_make_auth_query_t)GetProcAddress(handle, "client_make_auth_query");
    if (_pf_client_make_auth_query == NULL) sysdie("find symbol client_make_auth_query from {} failed. LastError={}"_format(filePath.c_str(), GetLastError()));
    client._f_client_make_auth_query = _pf_client_make_auth_query;

    auto _pf_client_deal_auth_reply = (_pf_client_deal_auth_reply_t)GetProcAddress(handle, "client_deal_auth_reply");
    if (_pf_client_deal_auth_reply == NULL) sysdie("find symbol client_deal_auth_reply from {} failed. LastError={}"_format(filePath.c_str(), GetLastError()));
    client._f_client_deal_auth_reply = _pf_client_deal_auth_reply;

    auto _pf_client_encode = (_pf_client_encode_t)GetProcAddress(handle, "client_encode");
    if (_pf_client_encode == NULL) sysdie("find symbol client_encode from {} failed. LastError={}"_format(filePath.c_str(), GetLastError()));
    client._f_client_encode = _pf_client_encode;

    auto _pf_client_decode = (_pf_client_decode_t)GetProcAddress(handle, "client_decode");
    if (_pf_client_decode == NULL) sysdie("find symbol client_decode from {} failed. LastError={}"_format(filePath.c_str(), GetLastError()));
    client._f_client_decode = _pf_client_decode;

    auto _pf_client_connection_close = (_pf_client_connection_close_t)GetProcAddress(handle, "client_connection_close");
    if (_pf_client_connection_close == NULL) sysdie("find symbol client_connection_close from {} failed. LastError={}"_format(filePath.c_str(), GetLastError()));
    client._f_client_connection_close = _pf_client_connection_close;
*/
    so_handle = handle; //Here is master thread. sysdie will exit program and windows will free this handle on error automatically.
                     //No need for scope guard.
    return;
}

#endif
