#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include "worker.h"
#include "client.h"
#include "dbg.h"
#include "event.h"
#include "server.h"

static void accept_client(EventSystem* es, TCPServer* server);
static void receive_from_client(EventSystem* es, HTTPClient* client);
static void send_to_client(EventSystem* es, HTTPClient* client);

void run_worker_process(TCPServer* server) {
    EventSystem* es = event_system_init();
    es_add(es, server->event.fd, server, EPOLLIN);

    // non-blocking event loop
    //while (true) {
        size_t nready = es_wait(es);

        for (size_t i = 0; i < nready; i++) {
            EventBase* event_data = (EventBase*) es->events[i].data.ptr;
            uint32_t events = es->events[i].events;
            
            switch (event_data->type) {
                case SERVER_EVENT:
                    accept_client(es, (TCPServer*) event_data);
                    break;
                case CLIENT_EVENT:
                    if (events & EPOLLIN) {
                        receive_from_client(es, (HTTPClient*) event_data);
                    } else if (events & EPOLLOUT) {
                        send_to_client(es, (HTTPClient*) event_data);
                    }
                    break;
            }
       }
   //}

    free(es);
    return ;
}

static void accept_client(EventSystem* es, TCPServer* server) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    int client_fd = accept(server->event.fd, (struct sockaddr*) &client_addr, &client_addr_size);

    if (client_fd == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        else
            log_err("Worker (PID: #%d) failed accepting connection", getpid());
    }

    log_info("Worker (PID: #%d) accepted request from client #%d", getpid(), client_fd);

    // todo: log client data

    make_non_blocking(client_fd);

    HTTPClient* client = http_client_init(client_fd);
    es_add(es, client_fd, client, EPOLLIN);
    log_info("Worker (PID: #%d) added client #%d to epoll", getpid(), client_fd);

    es_del(es, client_fd);
    log_info("Worker (PID: #%d) removed client #%d from epoll", getpid(), client_fd);

    close(client_fd);
    free(client);
    log_info("Worker (PID: #%d) closed connection with client #%d", getpid(), client_fd);
}

static void receive_from_client(EventSystem* es, HTTPClient* client) {
    log_info("receiving from client #%d", client->event.fd);
}

static void send_to_client(EventSystem* es, HTTPClient* client) {
    log_info("sending to client #%d", client->event.fd);
}
