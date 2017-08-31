#include "aes.h"
#include "kokke_aes.h"
#include <string.h>

size_t round_up(size_t base,size_t mult){
    return ((base/mult)*mult)+mult;
}

size_t round_down(size_t base,size_t mult){
    return ((base/mult)*mult);
}

void AESEncrypt(const char* src,const char* key,char* cip,size_t srclen,size_t keylen,uint8_t flag){
    if(flag){
        // mode_less
        // it's my duty to auto-pad your key...or not.
        char* rkey = malloc(16);
        if(keylen < 16){
            memcpy(rkey,key,keylen);
            memset(rkey+keylen,0,16-keylen);
        }else{
            memcpy(rkey,key,16);
        }
        memcpy(cip,src,srclen);
        AES_ECB_encrypt(src,rkey,cip,round_down(srclen,16));
        free(rkey);
    }else{
        // mode_fill
        char* rkey = malloc(16);
        if(keylen < 16){
            memcpy(rkey,key,keylen);
            memset(rkey+keylen,0,16-keylen);
        }else{
            memcpy(rkey,key,16);
        } 
        char delta = round_up(srclen,16)-srclen;
        char* rsrc = malloc(round_up(srclen,16));
        memset(rsrc,0,round_up(srclen,16));
        rsrc[round_up(srclen,16)-1]=delta;
        memcpy(rsrc,src,srclen);
        AES_ECB_encrypt(rsrc,rkey,cip,round_up(srclen,16));
        free(rsrc);
        free(rkey);
    }
}

size_t AESDecrypt(const char* src,const char* key,char* result,size_t srclen,size_t keylen,uint8_t flag){
    if(flag){
        // mode_less
        char* rkey = malloc(16);
        if(keylen < 16){
            memcpy(rkey,key,keylen);
            memset(rkey+keylen,0,16-keylen);
        }else{
            memcpy(rkey,key,16);
        }
        memcpy(result,src,srclen);
        AES_ECB_decrypt(src,rkey,result,round_down(srclen,16));
        free(rkey);
        return srclen;
    }else{
        // mode_fill
        char* rkey = malloc(16);
        if(keylen < 16){
            memcpy(rkey,key,keylen);
            memset(rkey+keylen,0,16-keylen);
        }else{
            memcpy(rkey,key,16);
        }
        AES_ECB_decrypt(src,rkey,result,srclen);
        free(rkey);
        // find out the real len
        return srclen-result[srclen-1];
    }
}