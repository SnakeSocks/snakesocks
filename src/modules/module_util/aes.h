#ifndef Q_ENCRYPTION_AES_H
#define Q_ENCRYPTION_AES_H

#include <stdint.h>
#include <stdlib.h>

size_t round_up(size_t base,size_t mult);
size_t round_down(size_t base,size_t mult);

#define Q_AES_FLAG_FILL 0
#define Q_AES_FLAG_LESS 1

// default ECB.
// Another method please use kokke_aes.h directly
void AESEncrypt(const char* src,const char* key,char* result,size_t srclen,size_t keylen,uint8_t flag);
size_t AESDecrypt(const char* src,const char* key,char* result,size_t srclen,size_t keylen,uint8_t flag);
// for fill mode encrypt:result shall have size at least round_up(len(src))
// for less mode encrypt:result len equals src len

// for decrypt: cip len == src len will works well with both mode.



#endif