// -*- related-file-name: "../../lib/selectset.cc" -*-
#ifndef CLICK_SELECTSET_HH
#define CLICK_SELECTSET_HH 1
#if !CLICK_USERLEVEL
# error "<click/selectset.hh> only meaningful at user level"
#endif
#include <click/vector.hh>
#include <click/sync.hh>
#include <unistd.h>
#if HAVE_POLL_H && !HAVE_USE_SELECT
# include <poll.h>
#endif
#if HAVE_SYS_EVENT_H && HAVE_KQUEUE && !HAVE_USE_SELECT && !HAVE_USE_POLL && !defined(HAVE_USE_KQUEUE)
# define HAVE_USE_KQUEUE 1
#elif (!HAVE_SYS_EVENT_H || !HAVE_KQUEUE) && HAVE_USE_KQUEUE
# error "--enable-select=kqueue is not supported on this system"
#endif
CLICK_DECLS
class Element;
class Router;
class RouterThread;

class SelectSet { public:

    SelectSet();
    ~SelectSet();

    void initialize();

    int add_select(int fd, Element *element, int mask);
    int remove_select(int fd, Element *element, int mask);

    void run_selects(RouterThread *thread);
    inline void wake();

    void kill_router(Router *router);

    inline void fence();

  private:

    struct SelectorInfo {
	Element *read;
	Element *write;
	int pollfd;
	SelectorInfo()
	    : read(0), write(0), pollfd(-1)
	{
	}
    };

    int _wake_pipe[2];
    volatile bool _wake_pipe_pending;
    RouterThread * volatile _blocked;
#if HAVE_USE_KQUEUE
    int _kqueue;
#endif
#if !HAVE_POLL_H || HAVE_USE_SELECT
    struct pollfd {
	int fd;
	int events;
    };
    fd_set _read_select_fd_set;
    fd_set _write_select_fd_set;
    int _max_select_fd;
#endif /* !HAVE_POLL_H || HAVE_USE_SELECT */
    Vector<struct pollfd> _pollfds;
    Vector<SelectorInfo> _selinfo;
#if HAVE_MULTITHREAD
    SimpleSpinlock _select_lock;
    click_processor_t _select_processor;
#endif

    void register_select(int fd, bool add_read, bool add_write);
    void remove_pollfd(int pi, int event);
    inline void call_selected(int fd, int mask) const;
    inline bool post_select(RouterThread *thread, bool acquire);
#if HAVE_USE_KQUEUE
    void run_selects_kqueue(RouterThread *thread);
#endif
#if HAVE_POLL_H && !HAVE_USE_SELECT
    void run_selects_poll(RouterThread *thread);
#else
    void run_selects_select(RouterThread *thread);
#endif

    inline void lock();
    inline void unlock();

};

inline void
SelectSet::wake()
{
    if (_blocked) {
	_wake_pipe_pending = true;
	ignore_result(write(_wake_pipe[1], "", 1));
    }
}

inline void
SelectSet::lock()
{
#if HAVE_MULTITHREAD
    if (click_get_processor() != _select_processor)
	_select_lock.acquire();
#endif
}

inline void
SelectSet::unlock()
{
#if HAVE_MULTITHREAD
    if (click_get_processor() != _select_processor)
	_select_lock.release();
#endif
}

inline void
SelectSet::fence()
{
    lock();
    unlock();
}

CLICK_ENDDECLS
#endif
