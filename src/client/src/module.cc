#include <cstring>
#include "module.hpp"
#include "syserr.hpp"
#include "NetLib.hpp"


binary_safe_string client_module::makeAuthQuery(connect_info *conn) const
{
    return _f_client_make_auth_query(conn);
}

bool client_module::onAuthReply(char *authdata, size_t len, connect_info *conn) const
{
    binary_safe_string server_reply;
    server_reply.null_terminated = false;
    server_reply.length = static_cast<uint32_t>(len);
    server_reply.str = (char *)authdata;

    return _f_client_deal_auth_reply(conn, server_reply);
}

binary_safe_string client_module::encode(binary_safe_string payload, connect_info *conn, client_query addrTemplate) const // MUST copy a new data.
{
    if(payload.length)
        addrTemplate.payload = payload;

    auto toReturn = _f_client_encode(conn, addrTemplate);
    return std::move(toReturn);
}

client_query client_module::decode(binary_safe_string payload, connect_info *conn) const
{
    auto toReturn = _f_client_decode(conn, payload);
    return std::move(toReturn);
}

#ifndef WIN32
//POSIX shared object
#include "module-port.hpp"
void client_module::loadSo(const string &filePath)
{
    RECORD
    using namespace ____trick;
    void *handle = dlopen(filePath.c_str(), RTLD_LAZY);
    char *errstr = NULL;
    if(!handle) sysdie("dlopen failed to open shared object %s, dlerror=%s", filePath.c_str(), dlerror());
    dlerror();

    auto _pf_client_make_auth_query = (_pf_client_make_auth_query_t)dlsym(handle, "client_make_auth_query");
    if ((errstr = dlerror()) != NULL) sysdie("find symbol client_make_auth_query from %s failed. dlerror=%s", filePath.c_str(), dlerror());
    _f_client_make_auth_query = _pf_client_make_auth_query;

    auto _pf_client_deal_auth_reply = (_pf_client_deal_auth_reply_t)dlsym(handle, "client_deal_auth_reply");
    if ((errstr = dlerror()) != NULL) sysdie("find symbol client_deal_auth_reply from %s failed. dlerror=%s", filePath.c_str(), dlerror());
    _f_client_deal_auth_reply = _pf_client_deal_auth_reply;

    auto _pf_client_encode = (_pf_client_encode_t)dlsym(handle, "client_encode");
    if ((errstr = dlerror()) != NULL) sysdie("find symbol client_encode from %s failed. dlerror=%s", filePath.c_str(), dlerror());
    _f_client_encode = _pf_client_encode;

    auto _pf_client_decode = (_pf_client_decode_t)dlsym(handle, "client_decode");
    if ((errstr = dlerror()) != NULL) sysdie("find symbol client_decode from %s failed. dlerror=%s", filePath.c_str(), dlerror());
    _f_client_decode = _pf_client_decode;
    
    auto _pf_client_connection_close = (_pf_client_connection_close_t)dlsym(handle, "client_connection_close");
    if ((errstr = dlerror()) != NULL) sysdie("find symbol client_connection_close from %s failed. dlerror=%s", filePath.c_str(), dlerror());
    _f_client_connection_close = _pf_client_connection_close;

    so_handle = handle; //Here is master thread. sysdie will exit program and linux will close this handle on error automatically.
                     //No need for scope guard.
    return;
}

#else
//Win32 dll
void client_module::loadSo(const string &filePath)
{
    using namespace ____trick;
    HMODULE handle = LoadLibrary(filePath.c_str());
    if(!handle) sysdie("Failed to open dll dylib %s, LastError=%d. Confirm if it exists and you have access to it.", filePath.c_str(), GetLastError());

    auto _pf_client_make_auth_query = (_pf_client_make_auth_query_t)GetProcAddress(handle, "client_make_auth_query");
    if (_pf_client_make_auth_query == NULL) sysdie("find symbol client_make_auth_query from %s failed. LastError=%d", filePath.c_str(), GetLastError());
    _f_client_make_auth_query = _pf_client_make_auth_query;

    auto _pf_client_deal_auth_reply = (_pf_client_deal_auth_reply_t)GetProcAddress(handle, "client_deal_auth_reply");
    if (_pf_client_deal_auth_reply == NULL) sysdie("find symbol client_deal_auth_reply from %s failed. LastError=%d", filePath.c_str(), GetLastError());
    _f_client_deal_auth_reply = _pf_client_deal_auth_reply;

    auto _pf_client_encode = (_pf_client_encode_t)GetProcAddress(handle, "client_encode");
    if (_pf_client_encode == NULL) sysdie("find symbol client_encode from %s failed. LastError=%d", filePath.c_str(), GetLastError());
    _f_client_encode = _pf_client_encode;

    auto _pf_client_decode = (_pf_client_decode_t)GetProcAddress(handle, "client_decode");
    if (_pf_client_decode == NULL) sysdie("find symbol client_decode from %s failed. LastError=%d", filePath.c_str(), GetLastError());
    _f_client_decode = _pf_client_decode;

    auto _pf_client_connection_close = (_pf_client_connection_close_t)GetProcAddress(handle, "client_connection_close");
    if (_pf_client_connection_close == NULL) sysdie("find symbol client_connection_close from %s failed. LastError=%d", filePath.c_str(), GetLastError());
    _f_client_connection_close = _pf_client_connection_close;

    so_handle = handle; //Here is master thread. sysdie will exit program and windows will free this handle on error automatically.
                     //No need for scope guard.
    return;
}

#endif