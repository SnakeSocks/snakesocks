#ifndef _SNAKESOCKS_PORT_HPP
#define _SNAKESOCKS_PORT_HPP 1

#include <string>
#include <functional>
#include <rlib/class_decorator.hpp>
#include "debug.hpp"
#include "module-port.hpp"

#if RLIB_OS_ID != OS_WINDOWS
#include <dlfcn.h>
#else
#include <windows.h>
#endif

using std::string;

class module_interface : private rlib::noncopyable
{
public:
    module_interface() = delete;
    module_interface(const string &soPath) {
        loadSo(soPath);
    }
    module_interface(module_interface &&ano)
        :
        client(std::move(ano.client)), 
        server(std::move(ano.server)),
        so_handle(ano.so_handle) {ano.so_handle = nullptr;}

   ~module_interface() {
        if(so_handle) 
#if RLIB_OS_ID != OS_WINDOWS
            dlclose(so_handle);
#else
            FreeLibrary(so_handle);
#endif
    }

    void loadSo(const string &filePath);

    //Functions below must be thread safe.
    struct client_impl {
        client_impl() {}
        client_impl(client_impl &&ano) :
            _f_client_make_auth_query (std::move(ano._f_client_make_auth_query)),
            _f_client_deal_auth_reply (std::move(ano._f_client_deal_auth_reply)),
            _f_client_encode          (std::move(ano._f_client_encode)),
            _f_client_decode          (std::move(ano._f_client_decode)),
            _f_client_connection_close(std::move(ano._f_client_connection_close)) {}
 
        binary_safe_string makeAuthQuery(connect_info *conn) const;
        bool onAuthReply(char *authdata, size_t len, connect_info *conn) const;
        binary_safe_string encode(binary_safe_string payload, connect_info *conn, client_query payload_template) const;
        client_query decode(binary_safe_string payload, connect_info *conn) const;
        void onCloseConn(connect_info *p) const;

        std::function<decltype(port_c::client_make_auth_query)> _f_client_make_auth_query;
        std::function<decltype(port_c::client_deal_auth_reply)> _f_client_deal_auth_reply;
        std::function<decltype(port_c::client_encode)> _f_client_encode;
        std::function<decltype(port_c::client_decode)> _f_client_decode;
        std::function<decltype(port_c::client_connection_close)> _f_client_connection_close;
    } client;
    struct server_impl {
        server_impl() {}
        server_impl(server_impl &&ano) :
            _f_server_make_auth_reply (std::move(ano._f_server_make_auth_reply)),
            _f_server_encode          (std::move(ano._f_server_encode)),
            _f_server_decode          (std::move(ano._f_server_decode)),
            _f_server_connection_close(std::move(ano._f_server_connection_close)) {}
 
        binary_safe_string makeAuthReply(connect_info *conn, binary_safe_string query, bool *output_auth_failed) const;
        binary_safe_string encode(connect_info *, client_query payload) const;
        client_query decode(connect_info *, binary_safe_string data) const;
        void onCloseConn(connect_info *) const;

        std::function<decltype(port_c::server_make_auth_reply)> _f_server_make_auth_reply;
        std::function<decltype(port_c::server_encode)> _f_server_encode;
        std::function<decltype(port_c::server_decode)> _f_server_decode;
        std::function<decltype(port_c::server_connection_close)> _f_server_connection_close;
    } server;

private:
#if RLIB_OS_ID != OS_WINDOWS
    void *so_handle;
#else
    HMODULE so_handle;
#endif
};

class server_module : private rlib::noncopyable
{
public:
};

#endif //_SNAKESOCKS_PORT_HPP
