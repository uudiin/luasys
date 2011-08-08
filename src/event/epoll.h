#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/inotify.h>

#define EVQ_SOURCE	"epoll.c"

#define NEVENT		64

#define EVENT_EXTRA							\
    struct event_queue *evq;

#define EVQ_EXTRA							\
    struct timeout_queue *tq;						\
    pthread_mutex_t cs;							\
    int volatile sig_ready;  /* triggered signals */			\
    int sig_fd;  /* eventfd to interrupt the loop */			\
    int epoll_fd;  /* epoll descriptor */

#endif
