#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

#include "dbg.h"
#include "server.h"
#include "master.h"

static void spawn_worker_processes(MasterProcess* master);

static TCPServer* g_server = NULL;

MasterProcess* master_process_init() {
    MasterProcess* master = (MasterProcess *)malloc(sizeof(MasterProcess));
    check_mem(master);

    master->pid = getpid();
    // todo: read workers count from config file
    master->workers_count = 16;

    g_server = tcp_server_init();
    tcp_server_start(g_server);

    return master;
error:
    exit(EXIT_FAILURE);
}

void run_master_process(MasterProcess* master) { 
    spawn_worker_processes(master);

    // todo: run ev loop; handle signals from child processes
    
    // temporary test
    sleep(10);
    return;
}

void free_master_process(MasterProcess* master) {
    tcp_server_stop(g_server);
    free(g_server);
    free(master);
}

static void spawn_worker_processes(MasterProcess* master) {
    for (size_t i = 0; i < master->workers_count; i++) {
        pid_t pid = fork();

        check(pid != -1, "Failed forking worker process");

        if (pid == 0) {
            pid_t w_pid = getpid();

            log_info("Worker #%ld (PID: %d) started working\n\n", i + 1, w_pid);

            // todo: run ev loop
            sleep(2);

            log_info("Worker #%ld (PID: %d) finished working\n\n", i + 1, w_pid);

            free_master_process(master);
            exit(EXIT_SUCCESS);
        }
    }

    return ;
error:
    exit(EXIT_FAILURE);
}
