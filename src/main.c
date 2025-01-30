#define _GNU_SOURCE
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "master.h"
#include "dbg.h"

uint16_t parse_port(int argc, char* argv[]);
uint16_t port_to_num(const char* port_str);

int main(int argc, char* argv[]) {
    uint16_t port = parse_port(argc, argv);

    MasterProcess* master = master_process_init(port);

    run_master_process(master);
    free_master_process(master);

    exit(EXIT_SUCCESS);
}

uint16_t parse_port(int argc, char* argv[]) {
    uint16_t port = 0;
    
    int opt;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
            case 'p':
                port = port_to_num(optarg);
                break;
        }
    }

    check(port != 0, "Usage: %s -p ${port}\n", argv[0]); 

    return port;
error:
    exit(EXIT_FAILURE);
}

uint16_t port_to_num(const char* port_str) {
    char* endptr;
    long port = strtol(port_str, &endptr, 10);
    
    check(*endptr == '\0' && port_str != endptr, "invalid port: '%s' - must be numeric\n", port_str);
    check(port >= 1 && port <= 65535, "port %ld out of range (1-65535)\n", port);
    
    return (uint16_t)port;
error:
    exit(EXIT_FAILURE);
}
