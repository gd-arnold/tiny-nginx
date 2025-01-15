#pragma once

#include <stdlib.h>

#include "event.h"

#define MAX_CLIENT_REQUEST_BUFFER 4096
#define MAX_CLIENT_HEADERS_BUFFER 1024

typedef enum ClientState {
    CLIENT_READING_REQUEST,
    CLIENT_SENDING_HEADERS,
    CLIENT_SENDING_FILE,
    CLIENT_SENDING_404,
    CLIENT_CLOSING
} ClientState;

typedef struct HTTPClient {
    EventBase event;

    ClientState state;

    char request[MAX_CLIENT_REQUEST_BUFFER];
    size_t request_len;
    
    char headers[MAX_CLIENT_HEADERS_BUFFER];
    size_t headers_len;
    size_t headers_sent;

    // todo: http request state
} HTTPClient;

HTTPClient* http_client_init(int fd, ClientState state);
