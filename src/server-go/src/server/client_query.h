#ifndef Q_INTERFACE_CQ_H
#define Q_INTERFACE_CQ_H

#include <stdint.h>
#include <stdlib.h>
#include "bss.h"

typedef struct _client_query {
    uint8_t destination_ip[16];
    uint16_t destination_port;
    binary_safe_string payload;// data pack to be sent to dest
}client_query;

#endif
