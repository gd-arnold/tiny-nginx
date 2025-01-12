#pragma once

#include <sys/epoll.h>

#define MAX_EVENTS 1024

typedef enum { SERVER_EVENT, CLIENT_EVENT } EventType;

typedef struct EventSystem {
    int epoll_fd;
    struct epoll_event events[MAX_EVENTS];
} EventSystem;

typedef struct EventBase {
    int fd;
    EventType type;
} EventBase;

EventSystem* event_system_init();

void es_add(EventSystem* es, EventBase* data, uint32_t events);
void es_mod(EventSystem* es, EventBase* data, uint32_t events);
void es_del(EventSystem* es, int fd);

size_t es_wait(EventSystem* es);

void make_non_blocking(int fd);
