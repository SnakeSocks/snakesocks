#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

typedef binary_safe_string (* _f_server_make_auth_reply_t)(connect_info *, binary_safe_string, bool *);
typedef binary_safe_string (* _f_server_encode_t)(connect_info *, client_query);
typedef client_query (* _f_server_decode_t)(connect_info *,binary_safe_string);
typedef void (* _f_server_connection_close_t)(connect_info *);


extern _f_server_make_auth_reply_t f_server_make_auth_reply;
extern _f_server_encode_t f_server_encode;
extern _f_server_decode_t f_server_decode;
extern _f_server_connection_close_t f_server_connection_close;

binary_safe_string call_server_make_auth_reply(connect_info * conf, binary_safe_string bss, bool * is_closing);
client_query call_server_decode(connect_info * conf,binary_safe_string data);
binary_safe_string call_server_encode(connect_info *conf, client_query payload);
void call_server_connection_close(connect_info *conf);
