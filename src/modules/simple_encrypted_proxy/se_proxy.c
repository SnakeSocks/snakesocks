// This is just a sample to tell you how to use api.
// You must not regard it as real c source code.
#include "../stdafx.h"
#include "../module_util/aes.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

// change the two str to random string in production environment for security
#define AUTH_STR "SE_PROXYv1"
#define MAGIC_STR "SE_MAGIC"

#define EXCEEDING_SIZE 512

struct _sym_passphrase{size_t pwlen;char str;};
typedef struct _sym_passphrase sym_passphrase;

bool strfullcmp(const char* a,const char* b,size_t alen,size_t blen){
    if(alen!=blen) return false;
    for(size_t i=0;i<alen;i++)
        if(a[i]!=b[i]) return false;
    return true;
}

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
    conn->additional_info = NULL;
    binary_safe_string query;
    query.null_terminated = false;
    query.length = round_up(strlen(AUTH_STR),16);
    query.str = malloc(query.length);
    memset(query.str,0,query.length);
    AESEncrypt(AUTH_STR,conn->passphrase.str,query.str,strlen(AUTH_STR),conn->passphrase.length,Q_AES_FLAG_FILL);
    return query;
}

bool client_deal_auth_reply(connect_info *conn, binary_safe_string server_reply) //Return true if auth is ok.
{
    //return isok(server_reply);
    // first decode the server reply
    char* result = malloc(server_reply.length);
    size_t rlen = AESDecrypt(server_reply.str,conn->passphrase.str,result,server_reply.length,conn->passphrase.length,Q_AES_FLAG_FILL);
    if(strncmp(result,MAGIC_STR,strlen(MAGIC_STR))){
        free(server_reply.str);
        return false;
    }
    // then the magic part is correct.
    // copy the new part as new passphrase for following encryption
    // cast to cancel the const desc
    size_t tmplen = rlen - strlen(MAGIC_STR);
    conn->additional_info = malloc(sizeof(sym_passphrase)+tmplen);
    memset(conn->additional_info,0,sizeof(sym_passphrase)+tmplen);
    sym_passphrase *ainfo = (sym_passphrase*)conn->additional_info; // cast for better reference
    ainfo->pwlen=tmplen;
    memcpy(&ainfo->str,result+strlen(MAGIC_STR),tmplen);
    free(server_reply.str);
    return true;
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
    /*uint8_t simple_encoder=bss_hash(conn->passphrase);
    for(size_t i=0;i<overall_size;i++){
        data[i]=data[i]^simple_encoder;
    }*/
    char* encresult = malloc(round_up(overall_size,16));
    memset(encresult,0,round_up(overall_size,16));
    AESEncrypt(data,&(((sym_passphrase*)(conn->additional_info))->str),encresult,overall_size,
               ((sym_passphrase*)(conn->additional_info))->pwlen,Q_AES_FLAG_FILL);
    free(data);
    binary_safe_string bss;
    bss.null_terminated = false;
    bss.length = round_up(overall_size,16);
    bss.str = encresult;
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
    /*uint8_t simple_encoder = bss_hash(conn->passphrase);
    for(size_t i =0;i<data.length;i++){
        data.str[i]=data.str[i]^simple_encoder;
    }*/
    char* tmpresult = malloc(data.length);
    data.length = AESDecrypt(data.str,&(((sym_passphrase*)(conn->additional_info))->str),tmpresult,data.length,
                             ((sym_passphrase*)(conn->additional_info))->pwlen,Q_AES_FLAG_FILL);
    free(data.str);
    data.str = tmpresult;
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
    /* now server will load conn->passphrase from config file
    conn->passphrase.length = data.length;
    conn->passphrase.null_terminated = data.null_terminated;
    conn->passphrase.str = malloc(data.length);
    memcpy(conn->passphrase.str,data.str,data.length);*/
    // check is fully correct
    // decode first
    char* request = malloc(data.length);
    size_t rlen = AESDecrypt(data.str,conn->passphrase.str,request,data.length,conn->passphrase.length,Q_AES_FLAG_FILL);
    if(!strfullcmp(request,AUTH_STR,rlen,strlen(AUTH_STR))){
        // failed.
        *is_closing = true;
        free(data.str);
        binary_safe_string bss;
        bss.null_terminated = false;
        bss.length = 1;
        bss.str = malloc(1);
        bss.str[0] = 1;
        return bss;
    }
    // success.
    // generate a random str
    srand(time(NULL));
    int tmprnd = rand();
    char* tmpenc = malloc(EXCEEDING_SIZE);
    size_t tmpenl = sprintf(tmpenc,"%s_%d",MAGIC_STR,tmprnd);
    binary_safe_string bss;
    bss.null_terminated = false;
    bss.str = malloc(round_up(tmpenl,16));
    bss.length = round_up(tmpenl,16);
    AESEncrypt(tmpenc,conn->passphrase.str,bss.str,tmpenl,conn->passphrase.length,Q_AES_FLAG_FILL);
    conn->additional_info = malloc(EXCEEDING_SIZE);
    memset(conn->additional_info,0,EXCEEDING_SIZE);
    sym_passphrase* ainfo = conn->additional_info;
    ainfo->pwlen = sprintf(&(ainfo->str),"_%d",tmprnd);
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
    /*uint8_t simple_encoder=bss_hash(conn->passphrase);
    for(size_t i=0;i<overall_size;i++){
        data[i]=data[i]^simple_encoder;
    }*/
    char* encresult = malloc(round_up(overall_size,16));
    memset(encresult,0,round_up(overall_size,16));
    AESEncrypt(data,&(((sym_passphrase*)(conn->additional_info))->str),encresult,overall_size,
               ((sym_passphrase*)(conn->additional_info))->pwlen,Q_AES_FLAG_FILL);
    free(data);
    binary_safe_string bss;
    bss.null_terminated = false;
    bss.length = round_up(overall_size,16);
    bss.str = encresult;
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
    /*uint8_t simple_encoder = bss_hash(conn->passphrase);
    for(size_t i =0;i<data.length;i++){
        data.str[i]=data.str[i]^simple_encoder;
    }*/
    char* tmpresult = malloc(data.length);
    data.length = AESDecrypt(data.str,&(((sym_passphrase*)(conn->additional_info))->str),tmpresult,data.length,
                             ((sym_passphrase*)(conn->additional_info))->pwlen,Q_AES_FLAG_FILL);
    free(data.str);
    data.str = tmpresult;
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
    if(conn->additional_info != NULL) free(conn->additional_info);
}

void server_connection_close(connect_info *conn){
    if(conn->additional_info != NULL) free(conn->additional_info);
}