#include <asm-generic/errno.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
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

static void receive_request(EventSystem* es, HTTPClient* client);

static void send_to_client(EventSystem* es, HTTPClient* client);
static void send_headers(EventSystem* es, HTTPClient* client);
static void send_file(EventSystem* es, HTTPClient* client);

static void close_client(EventSystem* es, HTTPClient* client);

static void parse_http_request(EventSystem* es, HTTPClient* client);
static void set_http_error_response(EventSystem* es, HTTPClient* client, const char* type);

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
                        free(es);
                        return;
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

    HTTPClient* client = http_client_init(fd, CLIENT_RECEIVING_REQUEST);
    es_add(es, (EventBase*) client, EPOLLIN);
    log_info("Worker (PID: #%d) added client #%d to epoll", getpid(), fd);

    return;
error:
    exit(EXIT_FAILURE);
}

static void receive_from_client(EventSystem* es, HTTPClient* client) {
    switch (client->state) {
        case CLIENT_RECEIVING_REQUEST:
            return receive_request(es, client);
        default:
            log_err("Invalid client (fd: %d) state %d", client->event.fd, getpid());
    }

}

static void send_to_client(EventSystem* es, HTTPClient* client) {
    switch (client->state) {
        case CLIENT_SENDING_HEADERS:
            return send_headers(es, client);
        case CLIENT_SENDING_FILE:
            return send_file(es, client);
        case CLIENT_CLOSING:
            return close_client(es, client);
        default:
            log_err("Invalid client (fd: %d) state %d", client->event.fd, getpid());
    }
}

static void receive_request(EventSystem* es, HTTPClient* client) {
    ssize_t bytes_received = 
        recv(client->event.fd, client->request + client->request_len,
                MAX_CLIENT_REQUEST_BUFFER - client->request_len - 1, 0);

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

    client->request_len += bytes_received;

    if (client->request_len >= MAX_CLIENT_REQUEST_BUFFER - 1) {
        // todo: send 413 http
        client->state = CLIENT_CLOSING;
        close_client(es, client);
        return;
    }

    client->request[client->request_len] = '\0';
    log_info("Worker (PID: #%d) received from client #%d", getpid(), client->event.fd);
    log_info("%s", client->request);

    if (strstr(client->request, "\r\n\r\n") != NULL) {
        parse_http_request(es, client);
    }
}

static void send_headers(EventSystem* es, HTTPClient* client) {
    ssize_t bytes_sent =
        send(client->event.fd, client->headers + client->headers_sent, 
                client->headers_len, 0);

    log_info("sending to client #%d", client->event.fd);

    if (bytes_sent == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        else
            log_err("Worker (PID: #%d) failed receiving from client #%d", getpid(), client->event.fd);
    }

    client->headers_sent += bytes_sent;

    if (client->headers_sent == client->headers_len) {
        // headers done sending, now send file
        if (client->http_error)
            client->state = CLIENT_CLOSING;
        else
            client->state = CLIENT_SENDING_FILE;

        send_to_client(es, client);
    }
}

static void send_file(EventSystem* es, HTTPClient* client) {
    // todo: send file
    log_info("sending file..");
    client->state = CLIENT_CLOSING;
    send_to_client(es, client);
}

static void close_client(EventSystem* es, HTTPClient* client) {
    es_del(es, client->event.fd);
    close(client->event.fd);
    free(client);
}

static void parse_http_request(EventSystem* es, HTTPClient* client) {
    char* request_line = strtok(client->request, "\r\n");

    if (request_line == NULL)
        return set_http_error_response(es, client, "400 Bad Request");

    const char* method = strtok(request_line, " ");
    const char* path = strtok(NULL, " ");
    const char* version = strtok(NULL, " ");

    if (method == NULL || path == NULL || version == NULL)
        return set_http_error_response(es, client, "400 Bad Request");

    if (strncmp(method, "GET", 3) != 0)
        return set_http_error_response(es, client, "501 Not Implemented");

    if (strncmp(version, "HTTP/1.", 7) != 0)
        return set_http_error_response(es, client, "505 HTTP Version Not Supported");

    // todo: process path

    const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\ntest";
    client->headers_len = strlen(response);
    memcpy(client->headers, response, client->headers_len);

    client->state = CLIENT_SENDING_HEADERS;
    es_mod(es, (EventBase*) client, EPOLLOUT);
}

static void set_http_error_response(EventSystem* es, HTTPClient* client, const char* type) {
    client->http_error = true;

    snprintf(client->headers, sizeof(client->headers),
            "HTTP/1.1 %s\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n\r\n",
            type);
    client->headers_len = strlen(client->headers);

    client->state = CLIENT_SENDING_HEADERS;
    es_mod(es, (EventBase*) client, EPOLLOUT);
}
