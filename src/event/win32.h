#ifndef EVQ_WIN32_H
#define EVQ_WIN32_H

#define EVQ_SOURCE	"win32.c"

#define NEVENT		(MAXIMUM_WAIT_OBJECTS-1)

/* Win32 Thread */
struct win32thr {
  struct event_queue *evq;
  struct timeout_queue *tq;
  struct win32thr *next, *next_ready;

  CRITICAL_SECTION sig_cs;
  HANDLE signal;
  unsigned int volatile n;  /* count of events */

#define WTHR_SLEEP	1
#define WTHR_POLL	2
#define WTHR_READY	3
#define WTHR_ACK	4
  unsigned int volatile state;

  unsigned int idx;  /* result of Wait* */
  HANDLE handles[NEVENT];  /* last handle is reserved for signal event */
  struct event *events[NEVENT-1];
};

/* Win32 NT I/O Completion Routines */
struct win32iocr {
  HANDLE h;  /* thread handle */
  struct win32overlapped *ov_head;  /* head of overlaps to be queued */
  struct win32overlapped *ov_tail;  /* tail of overlaps to be queued */
  struct win32overlapped * volatile ov_set;  /* set overlaps to queue */
  struct win32overlapped * volatile ov_ready;  /* ready overlaps */
};

struct win32overlapped {
  union {
    DWORD Internal;
    DWORD err;
  };
  union {
    DWORD InternalHigh;
    DWORD rw_flags;
  };
  union {
    LONGLONG Offset;
    struct win32overlapped *ov_next;
  };
  union {
    HANDLE hEvent;
    struct event *ev;
  };
};

#define EVENT_EXTRA							\
  struct win32thr *wth;							\
  union {								\
    unsigned int index;							\
    struct {								\
      struct win32overlapped *rov, *wov;  /* IOCR overlaps */		\
    } iocr;								\
  } w;

#define WIN32OV_BUF_IDX		6  /* initial buffer size on power of 2 */
#define WIN32OV_BUF_MAX		24  /* maximum buffer size on power of 2 */
#define WIN32OV_BUF_SIZE	(WIN32OV_BUF_MAX - WIN32OV_BUF_IDX + 1)

#define EVQ_EXTRA							\
  HANDLE ack_event;							\
  struct event *win_msg;  /* window messages handler */			\
  struct win32iocr iocr;						\
  struct win32thr * volatile wth_ready;					\
  struct win32thr head;							\
  int volatile nwakeup;  /* number of the re-polling threads */		\
  int volatile sig_ready;  /* triggered signals */			\
  int ov_buf_nevents;  /* number of used overlaps of cur. buffer */	\
  int ov_buf_index;  /* index of current buffer */			\
  struct win32overlapped *ov_free;  /* head of free overlaps */		\
  struct win32overlapped *ov_buffers[WIN32OV_BUF_SIZE];

#define event_get_evq(ev)	(ev)->wth->evq
#define event_get_tq_head(ev)	(ev)->wth->tq
#define event_deleted(ev)	((ev)->wth == NULL)
#define evq_is_empty(evq)	(!((evq)->nevents || (evq)->head.next))

/* Have to initialize the event source */
#define EVQ_POST_INIT

#define evq_post_init(ev)						\
  do {									\
    if (((ev)->flags & (EVENT_AIO | EVENT_PENDING | EVENT_ACTIVE)) == EVENT_AIO) \
      win32iocr_set((ev), (ev)->flags);					\
    else if ((ev)->flags & EVENT_DIRWATCH)				\
      FindNextChangeNotification((ev)->fd);				\
  } while (0)

EVQ_API int win32iocr_set (struct event *ev, const unsigned int ev_flags);

#endif
