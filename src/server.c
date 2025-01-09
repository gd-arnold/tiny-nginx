#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "dbg.h"
#include "event.h"
#include "server.h"

TCPServer* tcp_server_init() {
    TCPServer* server = (TCPServer*) malloc(sizeof(TCPServer));
    check_mem(server);

    server->port = 0;
    server->event.fd = 0;
    server->event.type = SERVER_EVENT;

    return server;
error:
    exit(EXIT_FAILURE);
}

void tcp_server_start(TCPServer* server) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    check(fd != -1, "Server socket creation failed");

    // make non-blocking
    make_non_blocking(fd);

    // todo: read port from config file
    uint16_t port = 3636;

    // server address on localhost:port
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    // bind address to socket
    int br = bind(fd, (struct sockaddr*) &address, sizeof(address));
    check(br != -1, "Failed binding server address to socket");

    // open socket
    int lr = listen(fd, SOMAXCONN);
    check(lr != -1, "Failed to open server socket");

    server->event.fd = fd;
    server->port = port;

    return;
error:
    exit(EXIT_FAILURE);
}

void tcp_server_stop(TCPServer* server) {
    close(server->event.fd);
    return;
}
