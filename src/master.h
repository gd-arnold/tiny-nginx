#pragma once

#include <stdint.h>
#include <sys/types.h>

#define MAX_WORKERS_COUNT 64

typedef struct MasterProcess {
    pid_t pid;
    pid_t w_pids[MAX_WORKERS_COUNT];
    uint8_t workers_count;
} MasterProcess;

MasterProcess* master_process_init();
void run_master_process(MasterProcess* master);
void free_master_process(MasterProcess* master);
