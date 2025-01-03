#include <stdlib.h>

#include "worker.h"
#include "dbg.h"

WorkerProcess* worker_process_init() {
    WorkerProcess* worker = (WorkerProcess*) malloc(sizeof(WorkerProcess));
    check_mem(worker);
    
    worker->pid = -1;

    return worker;
error:
    exit(EXIT_FAILURE);
}
