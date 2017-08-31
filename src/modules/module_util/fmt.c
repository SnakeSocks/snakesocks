#include "utils.h"
#include "../bss.h"
#include "../conn.h"
#include "../client_query.h"
#include <stdio.h>

void printbss(binary_safe_string bss,void (*prefix)(binary_safe_string bss),void (*aswhat)(char i),char div,void (*suffix)(binary_safe_string bss)){
    if(prefix != NULL) prefix(bss);
    for(size_t i=0;i<bss.length;i++){
        aswhat(bss.str[i]);
        if(i!=bss.length-1)
            if(div!=0)
                printf("%c",div);
    }
    if(suffix != NULL) suffix(bss);
}

void asnumber(char i){
    printf("%u",(unsigned int)((unsigned char)i));
}

void aschar(char i){
    printf("%c",i);
}

void lbracket(binary_safe_string bss){
    printf("[");
}

void rbracket(binary_safe_string bss){
    printf("]");
}