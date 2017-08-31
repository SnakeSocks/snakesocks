#include "utils.h"

#define OCTET_BIG 0
#define OCTET_LITTLE 1

int check_host_octet_order(void){
    uint32_t checker = 0x00ffff01;
    return (int)(((uint8_t*)&checker)[0]);
}

// network octet order is big 
void host_to_network_octet(void* dst,size_t len){
    if(!check_host_octet_order()) return;
    for(size_t i=0;i<len/2;i++){
        uint8_t* tmpdst = dst;
        uint8_t tmp = tmpdst[i];
        tmpdst[i]=tmpdst[len-1-i];
        tmpdst[len-1-i]=tmp;
    }
}

void network_to_host_octet(void* dst,size_t len){
    host_to_network_octet(dst,len);
}

uint16_t host_to_network_short(uint16_t a){
    uint16_t tmp = a;
    host_to_network_octet(&tmp,sizeof(uint16_t));
    return tmp;
}

uint16_t network_to_host_short(uint16_t a){
    return host_to_network_short(a);
}

uint32_t host_to_network_int(uint32_t a){
    uint32_t tmp = a;
    host_to_network_octet(&tmp,sizeof(tmp));
    return tmp;
}
uint32_t network_to_host_int(uint32_t a){
    return host_to_network_int(a);
}

uint64_t host_to_network_long(uint64_t a){
    uint64_t tmp = a;
    host_to_network_octet(&tmp,sizeof(tmp));
    return tmp;
}
uint64_t network_to_host_long(uint64_t a){
    return host_to_network_long(a);
}