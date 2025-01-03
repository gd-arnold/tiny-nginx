#pragma once

#include <sys/types.h>

typedef struct WorkerProcess {
    pid_t pid;
    // todo: cpuid (for cpu affinity)
} WorkerProcess;

WorkerProcess* worker_process_init();
