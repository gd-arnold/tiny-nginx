#include <stdlib.h>
#include <unistd.h>

#include "server.h"

int main(int argc, char* argv[]) {
    TCPServer* server = tcp_server_init();

    tcp_server_start(server);

    sleep(2);

    tcp_server_stop(server);

    free(server);
    exit(EXIT_SUCCESS);
}
