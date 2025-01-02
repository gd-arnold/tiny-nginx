#include <stdlib.h>
#include <unistd.h>

#include "master.h"

int main(int argc, char* argv[]) {
    MasterProcess* master = master_process_init();

    run_master_process(master);

    free_master_process(master);
    exit(EXIT_SUCCESS);
}
