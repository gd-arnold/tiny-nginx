/*
 * MIT License
 * 
 * Copyright (c) 2025 Georgi Arnaudov
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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

