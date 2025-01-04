#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "worker.h"
#include "dbg.h"

void run_worker_process(pid_t w_pid, TCPServer* server) {
    // todo: accept request (non-blocking)
    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    int client_fd = accept(server->socket_fd, (struct sockaddr*) &client_address, &client_address_size);

    check(client_fd != -1, "Worker (PID: #%d) failed accepting connection", w_pid);

    char buffer[1024] = "HTTP/1.1 200OK\r\n\r\ntest";
    send(client_fd, buffer, sizeof(buffer), 0);

    close(client_fd);

    log_info("Worker (PID: #%d) has served request to client. Terminating...", w_pid);
    return ;
error:
    exit(EXIT_FAILURE);
}
//WorkerProcess* worker_process_init() {
//    WorkerProcess* worker = (WorkerProcess*) malloc(sizeof(WorkerProcess));
//    check_mem(worker);
    
//    worker->pid = -1;

//    return worker;
//error:
//    exit(EXIT_FAILURE);
//}
