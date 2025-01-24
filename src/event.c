#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include "event.h"
#include "dbg.h"

EventSystem* event_system_init() {
    EventSystem* es = (EventSystem*) malloc(sizeof(EventSystem));
    check_mem(es);

    es->epoll_fd = epoll_create1(0);
    check(es->epoll_fd != -1, "Failed to create epoll instance");

    memset(es->events, 0, sizeof(es->events));

    return es;
error:
    exit(EXIT_FAILURE);
}

void es_add(EventSystem* es, EventBase* data, uint32_t events) {
    struct epoll_event event;
    event.data.ptr = data;
    event.events = events;

    int res = epoll_ctl(es->epoll_fd, EPOLL_CTL_ADD, data->fd, &event);
    check(res != -1, "Failed to add epoll event");

    return;
error:
    exit(EXIT_FAILURE);
}

void es_mod(EventSystem* es, EventBase* data, uint32_t events) {
    struct epoll_event event;
    event.data.ptr = data;
    event.events = events;

    int res = epoll_ctl(es->epoll_fd, EPOLL_CTL_MOD, data->fd, &event);
    check(res != -1, "Failed to modify epoll event");

    return;
error:
    exit(EXIT_FAILURE);

}

void es_del(EventSystem* es, int fd) {
    int res = epoll_ctl(es->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    check(res != -1, "Failed to delete epoll event");

    return;
error:
    exit(EXIT_FAILURE);
}

int es_wait(EventSystem* es) {
    return epoll_wait(es->epoll_fd, es->events, MAX_EVENTS, -1);
}

void make_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    check(flags != -1, "Failed getting flags of fd #%d", fd);

    int res = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    check(res != -1, "Failed making fd #%d non-blocking", fd);

    return;
error:
    exit(EXIT_FAILURE);
}
