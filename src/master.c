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
static void handle_shutdown_signal();

static void print_server_banner(MasterProcess* master);

static volatile sig_atomic_t g_running = true;

MasterProcess* master_process_init(uint16_t port) {
    MasterProcess* master = (MasterProcess *)malloc(sizeof(MasterProcess));
    check_mem(master);

    master->pid = getpid();
    memset(master->w_pids, 0, sizeof(master->w_pids));

    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    check(sched_getaffinity(0, sizeof(cpu_set), &cpu_set) != -1, "Failed retrieving cpu affinity mask");
    master->workers_count = CPU_COUNT(&cpu_set);

    master->server = tcp_server_init();
    tcp_server_start(master->server, port);

    return master;
error:
    exit(EXIT_FAILURE);
}

void run_master_process(MasterProcess* master) { 
    set_up_shutdown_signals();
    spawn_worker_processes(master);

    print_server_banner(master);
    
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
            int sr = sched_setaffinity(0, sizeof(child_cpu_set), &child_cpu_set);
            check(sr != -1, "Failed setting cpu affinity for worker #%ld (PID: %d)", i + 1, w_pid);

            run_worker_process(master->server);

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

static void handle_shutdown_signal() {
    g_running = false;
}

static void print_server_banner(MasterProcess* master) {
    const char* COLOR_GREEN = "\033[32m";
    const char* COLOR_BOLD_GREEN = "\033[1;32m";
    const char* COLOR_YELLOW = "\033[33m";
    const char* COLOR_BLUE = "\033[34m";
    const char* COLOR_CYAN = "\033[36m";
    const char* COLOR_RED = "\033[31m";
    const char* COLOR_RESET = "\033[0m";

    printf("%s%sTiny Nginx%s UP & RUNNING on %shttp://localhost:%d%s\n",
            COLOR_BOLD_GREEN, COLOR_GREEN, COLOR_RESET,
            COLOR_CYAN, master->server->port, COLOR_RESET);

    printf("%sNumber of workers spawned: %s%d%s\n",
            COLOR_YELLOW, COLOR_RESET, master->workers_count, COLOR_YELLOW);

    printf("%sServing files from %s%s%s\n\n",
            COLOR_BLUE, COLOR_RESET, PUBLIC_DIR, COLOR_BLUE);

    printf("%sPress [%sCtrl+C%s] to stop the server%s\n",
            COLOR_RESET, COLOR_RED, COLOR_RESET, COLOR_RESET);
}
