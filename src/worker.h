#pragma once

#include <sys/types.h>

#include "server.h"

//typedef struct WorkerProcess {
//    pid_t pid;
    // todo: cpuid (for cpu affinity)
//} WorkerProcess;

//WorkerProcess* worker_process_init();
void run_worker_process(pid_t w_pid, TCPServer* server);
