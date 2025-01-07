#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include "worker.h"
#include "dbg.h"
#include "event.h"
#include "master.h"

void run_worker_process(pid_t w_pid, TCPServer* server) {
    EventSystem* es = event_system_init();
    es_add(es, server->socket_fd, EPOLLIN);

    // main event loop
//    while (true) {
        size_t nready = es_wait(es);

        for (size_t i = 0; i < nready; i++) {
            int fd = es->events[i].data.fd;
            uint32_t events = es->events[i].events;
            
            if (fd == server->socket_fd) {
                // accept client
                struct sockaddr_in client_address;
                socklen_t client_address_size = sizeof(client_address);

                int client_fd = accept(server->socket_fd, (struct sockaddr*) &client_address, &client_address_size);
                check(client_fd != -1, "Worker (PID: #%d) failed accepting connection", w_pid);

                log_info("Worker (PID: #%d) has accepted request from client.", w_pid);

                char r_buffer[1024];
                memset(r_buffer, 0, sizeof(r_buffer));
                check(read(client_fd, r_buffer, sizeof(r_buffer) - 1) != -1, " ");
                log_info("REQUEST: %s", r_buffer);

                char buffer[1024] = "HTTP/1.1 200OK\r\n\r\ntest";
                send(client_fd, buffer, sizeof(buffer), 0);

                close(client_fd);

                log_info("Worker (PID: #%d) has served request to client. Closing connection.", w_pid);
                //
            } else if (events & EPOLLIN) {
                // receive from client
            } else if (events & EPOLLOUT) {
                // send to client
            }
        }
 //   }

    // todo: accept request (non-blocking)
 //   while (true) {
 //   }

    free(es);
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
