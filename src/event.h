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

#pragma once

#include <sys/epoll.h>

#define MAX_EVENTS 1024

typedef enum { SERVER_EVENT, CLIENT_EVENT, SIGNAL_EVENT } EventType;

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

int es_wait(EventSystem* es);
