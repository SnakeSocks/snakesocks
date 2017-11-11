#ifndef _SNAKESOCKS_PORT_HPP
#define _SNAKESOCKS_PORT_HPP 1

#include <string>
#include <functional>
#include <rlib/noncopyable.hpp>
#include "debug.hpp"
#include "module-port.hpp"

#ifndef WIN32
#include <dlfcn.h>
#else
#include <windows.h>
#endif

using std::string;

class client_module : private rlib::noncopyable
{
public:
    client_module() = delete;
    client_module(const string &soPath)
            {loadSo(soPath);}
    client_module(client_module &&ano)
            :
            _f_client_make_auth_query(std::move(ano._f_client_make_auth_query)),
            _f_client_deal_auth_reply(std::move(ano._f_client_deal_auth_reply)),
            _f_client_encode(std::move(ano._f_client_encode)),
            _f_client_decode(std::move(ano._f_client_decode)),
            _f_client_connection_close(std::move(ano._f_client_connection_close)),
            so_handle(ano.so_handle) {ano.so_handle = nullptr;}
    ~client_module() {
        if(so_handle) 
#ifndef WIN32
            dlclose(so_handle);
#else
            FreeLibrary(so_handle);
#endif
    }

    void loadSo(const string &filePath);

    //Functions below must be thread safe.
    binary_safe_string makeAuthQuery(connect_info *conn) const;
    bool onAuthReply(char *authdata, size_t len, connect_info *conn) const;
    binary_safe_string encode(binary_safe_string payload, connect_info *conn, client_query payload_template) const;
    client_query decode(binary_safe_string payload, connect_info *conn) const;
    void on_close_conn(connect_info *p) const {
        _f_client_connection_close(p);
    }
private:
    std::function<decltype(port_c::client_make_auth_query)> _f_client_make_auth_query;
    std::function<decltype(port_c::client_deal_auth_reply)> _f_client_deal_auth_reply;
    std::function<decltype(port_c::client_encode)> _f_client_encode;
    std::function<decltype(port_c::client_decode)> _f_client_decode;
    std::function<decltype(port_c::client_connection_close)> _f_client_connection_close;
#ifndef WIN32
    void *so_handle;
#else
    HMODULE so_handle;
#endif
};

#endif //_SNAKESOCKS_PORT_HPP
