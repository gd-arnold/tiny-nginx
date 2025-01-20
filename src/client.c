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

    client->file_fd = 0;

    return client;
error:
    exit(EXIT_FAILURE);
}
