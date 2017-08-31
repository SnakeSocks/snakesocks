#ifndef Q_MODULE_UTILS_H
#define Q_MODULE_UTILS_H

#include <stdint.h>
#include <stddef.h>
#include "../bss.h"


// octet related

void host_to_network_octet(void* dst,size_t len);
void network_to_host_octet(void* dst,size_t len);

uint16_t host_to_network_short(uint16_t a);
uint16_t network_to_host_short(uint16_t a);
uint32_t host_to_network_int(uint32_t a);
uint32_t network_to_host_int(uint32_t a);
uint64_t host_to_network_long(uint64_t a);
uint64_t network_to_host_long(uint64_t a);

// simple hash functions

unsigned int BKDRHash(const char* str,size_t len);

unsigned int BKDRHash_bss();

// printing functions

// this function can print binary safe string
// this stuff is pretty powerful but it can be difficult for use
// some support functions are provided so you can use it more easily
void printbss(binary_safe_string bss,void (*prefix)(binary_safe_string),void (*aswhat)(char),char div,void (*suffix)(binary_safe_string));
// support functions
void asnumber(char i);
void aschar(char i);
void lbracket(binary_safe_string);
void rbracket(binary_safe_string);
// so there are examples
// printbss(bss,lbracket,asnumber,' ',rbracket); -> [201 123 145 12 67]
// printbss(bss,NULL,aschar,0,NULL); -> abcdefg

#endif