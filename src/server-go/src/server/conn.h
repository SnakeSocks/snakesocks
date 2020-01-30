#ifndef Q_INTERFACE_CONN_H
#define Q_INTERFACE_CONN_H

#include <stdint.h>
#include <stdlib.h>
#include "bss.h"

typedef struct _connect_info
{
    uint8_t server_ip[16]; // notice both ip and port is encoded with network octet order.
    uint16_t server_port; // client_query also follows this regulation.
    const binary_safe_string passphrase; //Set from config file.
    int connect_fd; //Activating connection who call you.
    void *additional_info; //Hold other data that module need.
} connect_info;

#endif
