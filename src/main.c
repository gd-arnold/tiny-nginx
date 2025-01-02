#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "dbg.h"

int main(int argc, char* argv[]) {
    // tcp server init
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    check(server_fd != -1, "Server socket creation failed");

    // todo: make non-blocking

    // todo: read port from config file
    uint16_t port = 3636;

    // server address on localhost:port
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    // bind address to socket
    int br = bind(server_fd, (struct sockaddr*) &address, sizeof(address));
    check(br != -1, "Failed binding server address to socket");

    // open socket
    int lr = listen(server_fd, SOMAXCONN);
    check(lr != -1, "Failed to open server socket");

    sleep(5);

    close(server_fd);
    exit(EXIT_SUCCESS);
error:
    exit(EXIT_FAILURE);
}
