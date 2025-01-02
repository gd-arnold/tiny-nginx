#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "dbg.h"
#include "server.h"
#include "master.h"

static TCPServer* g_server = NULL;

MasterProcess* master_process_init() {
    MasterProcess* master = (MasterProcess *)malloc(sizeof(MasterProcess));
    check_mem(master);

    master->pid = getpid();
    // todo: read workers count from config file
    master->workers_count = 16;
    memset(master->workers, 0, sizeof(master->workers));

    g_server = tcp_server_init();
    tcp_server_start(g_server);

    return master;
error:
    exit(EXIT_FAILURE);
}

void run_master_process(MasterProcess* master) { 
    // todo: spawn worker processes

    // todo: run ev loop; handle signals from child processes
    
    // temporary test
    sleep(3);
}

void free_master_process(MasterProcess* master) {
    tcp_server_stop(g_server);
    free(g_server);
    free(master);
}
