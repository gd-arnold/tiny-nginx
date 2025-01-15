#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

#include "dbg.h"
#include "server.h"
#include "master.h"
#include "worker.h"

static void spawn_worker_processes(MasterProcess* master);
//static void handle_worker_exit(MasterProcess* master);
//static void handle_shutdown(MasterProcess* master);

MasterProcess* master_process_init() {
    MasterProcess* master = (MasterProcess *)malloc(sizeof(MasterProcess));
    check_mem(master);

    master->pid = getpid();
    memset(master->w_pids, 0, sizeof(master->w_pids));
    // todo: read workers count from config file
    master->workers_count = 3;

    master->server = tcp_server_init();
    tcp_server_start(master->server);

    return master;
error:
    exit(EXIT_FAILURE);
}

void run_master_process(MasterProcess* master) { 
    spawn_worker_processes(master);

    // todo: run ev loop; handle signals from child processes
    //while (true) {
        sleep(10);
    //}
    return;
}

void free_master_process(MasterProcess* master) {
    tcp_server_stop(master->server);
    free(master->server);
    free(master);
}

static void spawn_worker_processes(MasterProcess* master) {
    for (size_t i = 0; i < master->workers_count; i++) {
        pid_t pid = fork();
        check(pid != -1, "Failed forking worker process");

        master->w_pids[i] = pid;
        if (pid == 0) {
            pid_t w_pid = getpid();

            // todo: bind w_pid to a cpu

            log_info("Worker #%ld (PID: %d) started working\n\n", i + 1, w_pid);

            // todo: run ev loop
            run_worker_process(master->server);

            log_info("Worker #%ld (PID: %d) finished working\n\n", i + 1, w_pid);

            free_master_process(master);
            exit(EXIT_SUCCESS);
        }
    }

    return ;
error:
    exit(EXIT_FAILURE);
}
