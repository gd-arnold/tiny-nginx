#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <asm-generic/errno-base.h>
#include <sys/stat.h> 
#include <signal.h>
#include <asm-generic/errno.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/signalfd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <stdio.h>

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
static void set_http_error_headers(EventSystem* es, HTTPClient* client, const char* type);

static void resolve_path(EventSystem* es, HTTPClient* client, const char* path);
static void decode_url(const char* src, char* dst, size_t dst_size);

static const char* get_mime_type(const char* path);

static void set_up_signal_event(EventBase* e);

void run_worker_process(TCPServer* server) {
    bool running = true;

    EventBase signal;
    set_up_signal_event(&signal);
    
    EventSystem* es = event_system_init();
    es_add(es, (EventBase*) server, EPOLLIN);
    es_add(es, &signal, EPOLLIN);

    while (running) {
        int nready = es_wait(es);
        check(nready != -1, "Failed receiving epoll events");

        for (size_t i = 0; i < nready; i++) {
            EventBase* event_data = (EventBase*) es->events[i].data.ptr;
            uint32_t events = es->events[i].events;
            
            switch (event_data->type) {
                case SIGNAL_EVENT:
                    running = false;
                    break;
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

    close(signal.fd);
    close(es->epoll_fd);
    free(es);

    return;
error:
    exit(EXIT_FAILURE);
}

static void accept_client(EventSystem* es, TCPServer* server) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    int fd = accept(server->event.fd, (struct sockaddr*) &client_addr, &client_addr_size);

    if (fd == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            log_err("Worker (PID: #%d) failed accepting connection", getpid());

        return;
    }

    make_non_blocking(fd);

    int opt = 1;
    int or = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
    check(or != -1, "Worker (PID: #%d) failed setting TCP_NODELAY on client #%d", getpid(), fd);

    HTTPClient* client = http_client_init(fd, CLIENT_RECEIVING_REQUEST);
    es_add(es, (EventBase*) client, EPOLLIN);

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

        close_client(es, client);
    }

    // connection closed by client
    if (bytes_received == 0)
        return close_client(es, client);

    client->request_len += bytes_received;

    if (client->request_len >= MAX_CLIENT_REQUEST_BUFFER - 1)
        return set_http_error_headers(es, client, "413 Content Too Large");

    client->request[client->request_len] = '\0';

    if (strstr(client->request, "\r\n\r\n") != NULL) {
        parse_http_request(es, client);

        es_mod(es, (EventBase*) client, EPOLLOUT);
        client->state = CLIENT_SENDING_HEADERS;
    }
}

static void send_headers(EventSystem* es, HTTPClient* client) {
    ssize_t bytes_sent =
        send(client->event.fd, client->headers + client->headers_sent, 
                client->headers_len, 0);

    if (bytes_sent == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;

        client->state = CLIENT_CLOSING;
        return send_to_client(es, client);
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
    ssize_t n = sendfile(client->event.fd, client->file_fd,
            &client->file_offset,
            client->file_size - client->file_offset);

    if (n == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;

        client->state = CLIENT_CLOSING;
        return send_to_client(es, client);
    }

    
    if (client->file_offset == client->file_size) {
        client->state = CLIENT_CLOSING;
        send_to_client(es, client);
    }
}

static void close_client(EventSystem* es, HTTPClient* client) {
    if (client->file_fd != -1)
        close(client->file_fd);

    es_del(es, client->event.fd);
    close(client->event.fd);
    free(client);
}

static void parse_http_request(EventSystem* es, HTTPClient* client) {
    char* request_line = strtok(client->request, "\r\n");

    if (request_line == NULL)
        return set_http_error_headers(es, client, "400 Bad Request");

    const char* method = strtok(request_line, " ");
    const char* path = strtok(NULL, " ");
    const char* version = strtok(NULL, " ");

    if (method == NULL || path == NULL || version == NULL)
        return set_http_error_headers(es, client, "400 Bad Request");

    if (strncmp(method, "GET", 3) != 0)
        return set_http_error_headers(es, client, "501 Not Implemented");

    if (strncmp(version, "HTTP/1.", 7) != 0)
        return set_http_error_headers(es, client, "505 HTTP Version Not Supported");


    resolve_path(es, client, path);

    if (!client->http_error) {
        snprintf(client->headers, sizeof(client->headers),
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: %ld\r\n"
                "Content-Type: %s\r\n"
                "Connection: close\r\n\r\n",
                client->file_size, client->file_mime_type);

        client->headers_len = strlen(client->headers);
    }
}

static void set_http_error_headers(EventSystem* es, HTTPClient* client, const char* type) {
    client->http_error = true;

    snprintf(client->headers, sizeof(client->headers),
            "HTTP/1.1 %s\r\n"
            "Content-Length: 0\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n\r\n",
            type);
    client->headers_len = strlen(client->headers);
}

static void resolve_path(EventSystem* es, HTTPClient* client, const char* path) {
    char decoded_path[MAX_PATH_BUFFER];
    char full_path[MAX_PATH_BUFFER];
    char resolved_path[MAX_PATH_BUFFER];

    memset(decoded_path, 0, sizeof(decoded_path));
    memset(full_path, 0, sizeof(full_path));
    memset(resolved_path, 0, sizeof(resolved_path));

    decode_url(path, decoded_path, sizeof(decoded_path));

    // append index.html to directories
    if (decoded_path[strlen(decoded_path) - 1] == '/') {
        strncat(decoded_path, "index.html", sizeof(decoded_path) - strlen(decoded_path) - 1);
    }

    size_t public_dir_len = strlen(PUBLIC_DIR);
    size_t decoded_path_len = strlen(decoded_path);

    if (public_dir_len + 1 + decoded_path_len >= MAX_PATH_BUFFER)
        return set_http_error_headers(es, client, "414 URI Too Long");
    
    int written = snprintf(full_path, sizeof(full_path), "%s/%s", PUBLIC_DIR, decoded_path);
    if (written < 0 || (size_t)written >= sizeof(full_path))
       return set_http_error_headers(es, client, "414 URI Too Long");

    if (realpath(full_path, resolved_path) == NULL)
        return set_http_error_headers(es, client, "404 Not Found");

    if (strncmp(resolved_path, PUBLIC_DIR, public_dir_len) != 0 || 
            (resolved_path[public_dir_len] != '\0' && resolved_path[public_dir_len] != '/'))
        return set_http_error_headers(es, client, "404 Not Found");

    int file_fd = open(resolved_path, O_RDONLY);
    if (file_fd == -1)
        return set_http_error_headers(es, client, "404 Not Found");

    struct stat st;
    if (fstat(file_fd, &st) == -1 || !S_ISREG(st.st_mode)) {
        close(file_fd); 
        return set_http_error_headers(es, client, "404 Not Found");
    }

    client->file_fd = file_fd;
    client->file_size = st.st_size;
    client->file_mime_type = get_mime_type(resolved_path);
}

static void decode_url(const char* src, char* dst, size_t dst_size) {
    size_t src_len = strlen(src);

    for (size_t i = 0, j = 0; i < src_len && j < dst_size - 1; i++, j++) {
        if (src[i] == '%' && i + 2 < src_len) {
            unsigned char byte;
            sscanf(src + i + 1, "%2hhx", &byte); // decode %-encoded bytes
            dst[j] = byte;
            i += 2;
        } else if (src[i] == '+') {
            dst[j] = ' '; // convert '+' to space
        } else {
            dst[j] = src[i];
        }
    }
}

static const char* get_mime_type(const char* path) {
    const char *dot = strrchr(path, '.');
    if (!dot || dot == path) return "application/octet-stream";

    const char *ext = dot + 1;

    if (strcasecmp(ext, "html") == 0 || strcasecmp(ext, "htm") == 0)
        return "text/html";
    if (strcasecmp(ext, "css") == 0)
        return "text/css";
    if (strcasecmp(ext, "js") == 0)
        return "application/javascript";
    if (strcasecmp(ext, "json") == 0)
        return "application/json";
    if (strcasecmp(ext, "txt") == 0)
        return "text/plain";
    if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0)
        return "image/jpeg";
    if (strcasecmp(ext, "png") == 0)
        return "image/png";
    if (strcasecmp(ext, "gif") == 0)
        return "image/gif";
    if (strcasecmp(ext, "svg") == 0)
        return "image/svg+xml";
    if (strcasecmp(ext, "ico") == 0)
        return "image/x-icon";
    if (strcasecmp(ext, "woff") == 0)
        return "font/woff";
    if (strcasecmp(ext, "woff2") == 0)
        return "font/woff2";
    if (strcasecmp(ext, "ttf") == 0)
        return "font/ttf";
    if (strcasecmp(ext, "pdf") == 0)
        return "application/pdf";
    if (strcasecmp(ext, "zip") == 0)
        return "application/zip";
    
    return "application/octet-stream";
}

static void set_up_signal_event(EventBase* e) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);

    int sm = sigprocmask(SIG_BLOCK, &mask, NULL);
    check(sm != -1, "Failed setting SIG_BLOCK in signal mask (needed for signalfd)");

    int fd = signalfd(-1, &mask, 0);
    check(fd != -1, "Failed creating signalfd");

    e->fd = fd;
    e->type = SIGNAL_EVENT;
    return;
error:
    exit(EXIT_FAILURE);
}
