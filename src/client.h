/*
 * MIT License
 * 
 * Copyright (c) 2025 Georgi Arnaudov
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "event.h"

#define MAX_CLIENT_REQUEST_BUFFER 4096
#define MAX_CLIENT_HEADERS_BUFFER 1024

#define MAX_PATH_BUFFER 4096

typedef enum ClientState {
    CLIENT_RECEIVING_REQUEST,
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

    bool http_error;

    int file_fd;
    off_t file_offset;
    size_t file_size;
    const char* file_mime_type;
} HTTPClient;

HTTPClient* http_client_init(int fd, ClientState state);
