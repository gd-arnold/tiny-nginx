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

#include <stdlib.h>

#include "client.h"
#include "dbg.h"
#include "event.h"

HTTPClient* http_client_init(int fd, ClientState state) {
    HTTPClient* client = (HTTPClient*) malloc(sizeof(HTTPClient));
    check_mem(client);

    client->event.fd = fd;
    client->event.type = CLIENT_EVENT;

    client->state = state;

    memset(client->request, 0, sizeof(client->request));
    client->request_len = 0;

    memset(client->headers, 0, sizeof(client->headers));
    client->headers_len = 0;
    client->headers_sent = 0;

    client->http_error = false;

    client->file_fd = -1;
    client->file_offset = 0;
    client->file_size = 0;
    client->file_mime_type = NULL;

    return client;
error:
    exit(EXIT_FAILURE);
}
