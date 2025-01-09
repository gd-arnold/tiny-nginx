#pragma once

#include <stdint.h>

#include "event.h"

typedef struct TCPServer {
    EventBase event;
    uint16_t port;
} TCPServer;

TCPServer* tcp_server_init();
void tcp_server_start(TCPServer* server);
void tcp_server_stop(TCPServer* server);
