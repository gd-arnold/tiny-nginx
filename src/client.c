#include <stdlib.h>

#include "client.h"
#include "dbg.h"

HTTPClient* http_client_init(int fd) {
    HTTPClient* client = (HTTPClient*) malloc(sizeof(HTTPClient));
    check_mem(client);

    client->fd = fd;

    memset(client->read, 0, sizeof(client->read));
    client->read_len = 0;

    memset(client->write, 0, sizeof(client->write));
    client->write_len = 0;
    client->write_sent = 0;

    return client;
error:
    exit(EXIT_FAILURE);
}
