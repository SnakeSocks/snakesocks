#ifndef _SNAKESOCKS_STDAFX_H
#define _SNAKESOCKS_STDAFX_H 1
// All SnakeSocks module must include this header, and implement every function listed below.
// You can use any language freely in your source. But C-style function must be exposed directly
// in compiled object, so that SnakeSocks can import and find needed function by symbol.

// NOTICE: DO NOT FORGET TO CHECK IF STRING_LEN == 0 IF NECESSARY.

#ifdef __cplusplus
extern "C" {
#endif


// Common definition
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
/*typedef struct _binary_safe_string
{
    bool null_terminated; //If true, it's OK to either regard str as a null-terminated string, or as a binary_safe_string.
    uint32_t length; //length(empty_string) must be 0.
    char *str; //This string must be created by malloc/calloc, released by free. Last user must free it.
} binary_safe_string;
typedef struct _connect_info
{
    uint8_t server_ip[16]; // notice both ip and port is encoded with network octet order.
    uint16_t server_port; // client_query also follows this regulation.
    binary_safe_string passphrase; //Set from config file.
    int connect_fd; // Activating connection who call you. WARNING : DO NOT execute any operation on the fd given here.
                    // There is no guarantee that any operation applied at this stuff will work or not.
                    // Future version might remove this entry. Suggestions:DO NOT CODE ANYTHING USING THIS STUFF.
    void *additional_info; //Save some data for connection if you'd like to. Client and server will do nothing to it.
} connect_info;
typedef struct _client_query {
    uint8_t destination_ip[16];
    uint16_t destination_port;
    binary_safe_string payload;// data pack to be sent to dest
}client_query;*/

// updated file structure for better util programming.

#include "bss.h"
#include "conn.h"
#include "client_query.h"
/* Client-side function declaration --
 *
 * Agreement about malloc and free to binary_safe_string::str:
 *
 * You must NEVER `free` str inside connect_info passed to you.
 * You must NEVER try to edit binary_safe_string inside connect_info. Regard them as const char * please.
 *
 * For `client_deal_auth_reply`:
 * You must NEVER free server_reply.
 *
 * For `client_encode` and `client_decode`:
 * You can either malloc/calloc a new binary_safe_string to return and free `data`, or edit `data` and return it.
 * I'll free returned string only once, and never free `data` passed to client_(en/de)code.
 */
binary_safe_string client_make_auth_query(connect_info *);
bool client_deal_auth_reply(connect_info *, binary_safe_string server_reply); //Return true if auth is ok.
binary_safe_string client_encode(connect_info *,client_query payload); //I'll call it per complete packet.
client_query client_decode(connect_info *, binary_safe_string data); //I'll call it per complete packet.
void client_connection_close(connect_info *); //Be caution: this function may be called before client_deal_auth_reply().

// Server-side function declaration
// The agreement is mainly the same as client-side function.
// For `server_make_auth_reply`:
// You must free the binary_safe_string pass to you, I'll never free it.
binary_safe_string server_make_auth_reply(connect_info *, binary_safe_string, bool * is_closing);
binary_safe_string server_encode(connect_info *, client_query payload);
client_query server_decode(connect_info *,binary_safe_string data);
void server_connection_close(connect_info *); //Be caution: this function may be called before server_make_auth_reply().

// -------------------------------------------------------------------------------------
// functions behind this comment is the next api to be implemented and used in
// protocol definition on snakesocks.
// will not be implemented in a short time, but once it can be correctly accessed
// by the main program of snakesocks, this comment will be removed.
// -------------------------------------------------------------------------------------
// the foundamental idea of apis provided behind is : to provide more widely ranged
// control of the protocol we use to communicating between the server and client.
// In the first snakesocks c/s version, we forced network packs to have an leading
// "size" to ensure all specified strings to be decoded correctly.
// But in the following version, which is the following apis will apply on,
// the users can have fully control on how we define a "package".
// for example, you can use "terminating octet serial" instead of "leading size number"
// to divide a stream into packages(like HTTP protocol does).
// Even if you found that's a bad idea, there are solutions for you to control how
// we encode the "leading size number"(in the current version, we just sent it barely),
// and the length of it.
// -------------------------------------------------------------------------------------

// API : tell us which method you are using:"terminating octet serial" or "leading size number"
uint8_t select_dividing_method(connect_info *conn);

// API : generating terminating octet serial
// note: the appending work(append terminating octets to data to be sent) will be done automatically by us
binary_safe_string generate_terminating_serial(connect_info *conn,binary_safe_string tobesent);

// API : check terminating octet serial
// explain: this process will work like the following:
// the server/client will read data byte by byte, and first put one byte in the second parameter;
// then check your return value:
// on -1: not correct. the data in octet_serial will be seen as part of package data.
//        contents in the octet_serial will be storaged by us and will not appear again in next call.
// on  0: correct but not complete. that means the data in octet_serial assembled a part of terminating
//        serial. we will append next byte we read in it and retry this api until hit a return of -1 or 1.
// on  1: terminated. that means the data in octet serial fully matched with the terminating octet serial
//        defined by you. the data in octet_serial will be discarded and the data saved before will be
//        treated as a completed package.
int8_t check_octets(connect_info *conn,uint8_t *octet_serial);

// API : assemble the size number:
binary_safe_string generate_size_number(connect_info *conn,size_t the_size);

// API : the length of size number generated:
size_t size_number_len(connect_info *conn);

// API : decode the size number
size_t decode_size_number(connect_info *conn,binary_safe_string the_size);


#ifdef __cplusplus
};
#endif

#endif //_SNAKESOCKS_STDAFX_H
