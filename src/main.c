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
