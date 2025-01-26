#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sched.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

#include "dbg.h"
#include "server.h"
#include "master.h"
#include "worker.h"

static void spawn_worker_processes(MasterProcess* master);

static void set_up_shutdown_signals();
static void handle_shutdown_signal(int sig);

static volatile sig_atomic_t g_running = true;

MasterProcess* master_process_init() {
    MasterProcess* master = (MasterProcess *)malloc(sizeof(MasterProcess));
    check_mem(master);

    master->pid = getpid();
    memset(master->w_pids, 0, sizeof(master->w_pids));

    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    check(sched_getaffinity(0, sizeof(cpu_set), &cpu_set) != -1, "Failed retrieving cpu affinity mask");
    master->workers_count = CPU_COUNT(&cpu_set);

    master->server = tcp_server_init();
    tcp_server_start(master->server);

    return master;
error:
    exit(EXIT_FAILURE);
}

void run_master_process(MasterProcess* master) { 
    set_up_shutdown_signals();
    spawn_worker_processes(master);

    // block until shutdown signal requested
    while (g_running) {
        sleep(1);
    }

    // send SIGTERM to all workers
    kill(0, SIGTERM);
    
    // wait until workers terminate
    while (wait(NULL) > 0) {}
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

            // bind w_pid to a cpu
            cpu_set_t child_cpu_set;
            CPU_ZERO(&child_cpu_set);
            CPU_SET(i, &child_cpu_set);

            check(sched_setaffinity(0, sizeof(child_cpu_set), &child_cpu_set) != -1,
                    "Failed setting cpu affinity for worker #%ld (PID: %d)", i + 1, w_pid);

            log_info("Worker #%ld (PID: %d) started working\n\n", i + 1, w_pid);

            run_worker_process(master->server);

            log_info("Worker #%ld (PID: %d) finished working\n\n", i + 1, w_pid);

            free_master_process(master);
            exit(EXIT_SUCCESS);
        }
    }

    return;
error:
    exit(EXIT_FAILURE);
}

static void set_up_shutdown_signals() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = handle_shutdown_signal;

    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
}

static void handle_shutdown_signal(int sig) {
    g_running = false;
}

