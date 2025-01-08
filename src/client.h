#pragma once

#include <stdlib.h>

#define MAX_CLIENT_READ_BUF 4096
#define MAX_CLIENT_WRITE_BUF 4096

typedef struct HTTPClient {
    int fd;

    // read state
    char read[MAX_CLIENT_READ_BUF];
    size_t read_len;
    
    // write state
    char write[MAX_CLIENT_WRITE_BUF];
    size_t write_len;
    size_t write_sent;

    // todo: http request state
} HTTPClient;

HTTPClient* http_client_init(int fd);
