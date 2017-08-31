// This is just a sample to tell you how to use api.
// You must not regard it as real c source code.
#include "../stdafx.h"
#include <stdlib.h>
#include <string.h>

uint8_t simple_hash(const char* str,size_t len){
    uint8_t tmp=0;
    for(size_t i = 0;i<len;i++){
        tmp+=str[i];
    }
    return tmp;
}

uint8_t bss_hash(binary_safe_string bss){
    return simple_hash(bss.str,bss.length);
}

// Client-side function implement.
binary_safe_string client_make_auth_query(connect_info *conn)
{
    /*bstr = malloc();
    do_something(sinf, bstr);
    return bstr;*/
    binary_safe_string query;
    query.null_terminated = false;
    query.length = conn->passphrase.length;
    query.str = malloc(query.length);
    memcpy(query.str,conn->passphrase.str,query.length);
    return query;
}

bool client_deal_auth_reply(connect_info *conn, binary_safe_string server_reply) //Return true if auth is ok.
{
    //return isok(server_reply);
    return server_reply.str[0];
}
binary_safe_string client_encode(connect_info *conn, client_query payload)
{
    struct data_struct {uint8_t dstip[16];uint16_t dstport;uint32_t plsz;uint8_t payload;};
    size_t overall_size = sizeof(struct data_struct)+payload.payload.length-1;
    uint8_t* data = malloc(overall_size);
    // construct data
    memcpy(data,&payload,sizeof(uint8_t)*16);
    ((struct data_struct*)data)->dstport = payload.destination_port;
    ((struct data_struct*)data)->plsz = payload.payload.length;
    if(payload.payload.length != 0){
        memcpy(&(((struct data_struct*)data)->payload),payload.payload.str,payload.payload.length);
    }
    uint8_t simple_encoder=bss_hash(conn->passphrase);
    for(size_t i=0;i<overall_size;i++){
        data[i]=data[i]^simple_encoder;
    }
    binary_safe_string bss;
    bss.null_terminated = false;
    bss.length = overall_size;
    bss.str =(char*)data;
    free(payload.payload.str);
    return bss;
}
client_query client_decode(connect_info *conn, binary_safe_string data)
{
    /*if(somecond)
    {
        return do_something(data);
    }
    else
    {
        bstr = malloc();
        do_something(data, bstr);
        free(data);
        return bstr;
    }*/
    struct data_struct {uint8_t dstip[16];uint16_t dstport;uint32_t plsz;uint8_t payload;};
    uint8_t simple_encoder = bss_hash(conn->passphrase);
    for(size_t i =0;i<data.length;i++){
        data.str[i]=data.str[i]^simple_encoder;
    }
    client_query query;
    query.payload.length = 0;
    query.payload.str = NULL;
    if(data.length <= 24) {free(data.str);return query;}// not correct.
    memcpy(&query,data.str,sizeof(uint8_t)*16);
    query.destination_port = ((struct data_struct*)data.str)->dstport;
    query.payload.null_terminated = false;
    query.payload.length = ((struct data_struct*)data.str)->plsz;
    if(query.payload.length != 0){
        query.payload.str = malloc(query.payload.length);
        memcpy(query.payload.str,&(((struct data_struct*)data.str)->payload),query.payload.length);
    }else{
        query.payload.str=NULL;
    }
    free(data.str);
    return query;

}

// Server-side function implement.
binary_safe_string server_make_auth_reply(connect_info *conn, binary_safe_string data, bool * is_closing)
{
    /*bstr = malloc();
    *is_closing = do_something(sinf,data,bstr);
    return bstr;*/
    binary_safe_string bss;
    bss.null_terminated = false;
    bss.length = 1;
    bss.str = malloc(1);
    bss.str[0] = 1;
    *is_closing = false;
    free(data.str);
    return bss;
}
binary_safe_string server_encode(connect_info *conn, client_query payload)
{
    /*if(somecond)
    {
        return do_something(data);
    }
    else
    {
        bstr = malloc();
        do_something(data, bstr);
        free(data);
        return bstr;
    }*/
    struct data_struct {uint8_t dstip[16];uint16_t dstport;uint32_t plsz;uint8_t payload;};
    size_t overall_size = sizeof(struct data_struct)+payload.payload.length-1;
    uint8_t* data = malloc(overall_size);
    // construct data
    memcpy(data,&payload,sizeof(uint8_t)*16);
    ((struct data_struct*)data)->dstport = payload.destination_port;
    ((struct data_struct*)data)->plsz = payload.payload.length;
    if(payload.payload.length != 0){
        memcpy(&(((struct data_struct*)data)->payload),payload.payload.str,payload.payload.length);
    }
    uint8_t simple_encoder=bss_hash(conn->passphrase);
    for(size_t i=0;i<overall_size;i++){
        data[i]=data[i]^simple_encoder;
    }
    binary_safe_string bss;
    bss.null_terminated = false;
    bss.length = overall_size;
    bss.str =(char*)data;
    free(payload.payload.str);
    return bss;
}
client_query server_decode(connect_info *conn, binary_safe_string data)
{
    /*if(somecond)
    {
        return do_something(data);
    }
    else
    {
        bstr = malloc();
        do_something(data, bstr);
        free(data);
        return bstr;
    }*/
    struct data_struct {uint8_t dstip[16];uint16_t dstport;uint32_t plsz;uint8_t payload;};
    uint8_t simple_encoder = bss_hash(conn->passphrase);
    for(size_t i =0;i<data.length;i++){
        data.str[i]=data.str[i]^simple_encoder;
    }
    client_query query;
    query.payload.length = 0;
    query.payload.str = NULL;
    if(data.length <= 24) {free(data.str);return query;}// not correct.
    memcpy(&query,data.str,sizeof(uint8_t)*16);
    query.destination_port = ((struct data_struct*)data.str)->dstport;
    query.payload.null_terminated = false;
    query.payload.length = ((struct data_struct*)data.str)->plsz;
    if(query.payload.length != 0){
        query.payload.str = malloc(query.payload.length);
        memcpy(query.payload.str,&(((struct data_struct*)data.str)->payload),query.payload.length);
    }else{
        query.payload.str = NULL;
    }
    free(data.str);
    return query;
}

void client_connection_close(connect_info *conn){
    // do nothing.
}

void server_connection_close(connect_info *conn){
    // do nothing.
}
