#include <asm-generic/errno.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
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

static void close_client(EventSystem* es, HTTPClient* client);

void run_worker_process(TCPServer* server) {
    EventSystem* es = event_system_init();
    es_add(es, (EventBase*) server, EPOLLIN);

    // non-blocking event loop
    while (true) {
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
   }

    free(es);
    return ;
}

static void accept_client(EventSystem* es, TCPServer* server) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    int fd = accept(server->event.fd, (struct sockaddr*) &client_addr, &client_addr_size);

    if (fd == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        else
            log_err("Worker (PID: #%d) failed accepting connection", getpid());
    }

    log_info("Worker (PID: #%d) accepted request from client #%d", getpid(), fd);

    // todo: log client data

    make_non_blocking(fd);

    int opt = 1;
    int or = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
    check(or != -1, "Worker (PID: #%d) failed setting TCP_NODELAY on client #%d", getpid(), fd);

    HTTPClient* client = http_client_init(fd);
    es_add(es, (EventBase*) client, EPOLLIN);
    log_info("Worker (PID: #%d) added client #%d to epoll", getpid(), fd);

    return;
error:
    exit(EXIT_FAILURE);
}

static void receive_from_client(EventSystem* es, HTTPClient* client) {
    ssize_t bytes_received = 
        recv(client->event.fd, client->read + client->read_len,
                MAX_CLIENT_READ_BUF - client->read_len - 1, 0);

    if (bytes_received == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        else
            log_err("Worker (PID: #%d) failed receiving from client #%d", getpid(), client->event.fd);
    }

    // connection closed by client
    if (bytes_received == 0) {
        close_client(es, client);
        return;
    }

    client->read_len += bytes_received;

    if (client->read_len >= MAX_CLIENT_READ_BUF - 1) {
        // todo: send 413 http
        close_client(es, client);
        return;
    }

    client->read[client->read_len] = '\0';
    log_info("Worker (PID: #%d) received from client #%d", getpid(), client->event.fd);
    log_info("%s", client->read);

    if (strstr(client->read, "\r\n\r\n") != NULL) {
        // todo: process request
        close_client(es, client);
    }
}

static void send_to_client(EventSystem* es, HTTPClient* client) {
    log_info("sending to client #%d", client->event.fd);
}

static void close_client(EventSystem* es, HTTPClient* client) {
    es_del(es, client->event.fd);
    close(client->event.fd);
    free(client);
}
