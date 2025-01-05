#pragma once

#include <sys/epoll.h>

#define MAX_EVENTS 1024

typedef struct EventSystem {
    int epoll_fd;
    struct epoll_event events[MAX_EVENTS];
} EventSystem;

EventSystem* event_system_init();

void add(EventSystem* es, int fd, uint32_t events);
void mod(EventSystem* es, int fd, uint32_t events);
void del(EventSystem* es, int fd, uint32_t events);

size_t wait(EventSystem* es);
