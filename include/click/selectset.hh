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

    int add_select(int fd, Element *element, int mask);
    int remove_select(int fd, Element *element, int mask);

    void run_selects(RouterThread *thread);

    void kill_router(Router *router);

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
    Spinlock _select_lock;

#if HAVE_MULTITHREAD
    static RouterThread * volatile selecting_thread;
#endif

    void register_select(int fd, bool add_read, bool add_write);
    void remove_pollfd(int pi, int event);
    inline void call_selected(int fd, int mask) const;
#if HAVE_USE_KQUEUE
    void run_selects_kqueue(RouterThread *thread, bool more_tasks);
#endif
#if HAVE_POLL_H && !HAVE_USE_SELECT
    void run_selects_poll(RouterThread *thread, bool more_tasks);
#else
    void run_selects_select(RouterThread *thread, bool more_tasks);
#endif

    friend class Master;	// for _select_lock

};

CLICK_ENDDECLS
#endif
