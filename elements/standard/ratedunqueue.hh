// -*- c-basic-offset: 4 -*-
#ifndef CLICK_RATEDUNQUEUE_HH
#define CLICK_RATEDUNQUEUE_HH
#include <click/element.hh>
#include <click/tokenbucket.hh>
#include <click/task.hh>
#include <click/timer.hh>
#include <click/notifier.hh>
CLICK_DECLS

/*
 * =c
 * RatedUnqueue(RATE)
 * =s shaping
 * pull-to-push converter
 * =d
 *
 * Pulls packets at the given RATE in packets per second, and pushes them out
 * its single output.
 *
 * =h rate read/write
 *
 * =a BandwidthRatedUnqueue, Unqueue, Shaper, RatedSplitter */

class RatedUnqueue : public Element { public:

    RatedUnqueue();
    ~RatedUnqueue();

    const char *class_name() const	{ return "RatedUnqueue"; }
    const char *port_count() const	{ return PORTS_1_1; }
    const char *processing() const	{ return PULL_TO_PUSH; }
    bool is_bandwidth() const		{ return class_name()[0] == 'B'; }

    int configure(Vector<String> &, ErrorHandler *);
    bool can_live_reconfigure() const	{ return true; }
    int initialize(ErrorHandler *);
    void add_handlers();

    bool run_task(Task *);

  protected:

    TokenBucket _rate;
    unsigned _rate_raw;
    Task _task;
    Timer _timer;
    NotifierSignal _signal;
    uint32_t _runs;
    uint32_t _pushes;
    uint32_t _failed_pulls;
    uint32_t _empty_runs;

    enum {h_calls, h_rate};

    static String read_handler(Element *e, void *thunk);

    bool _active;
};

CLICK_ENDDECLS
#endif
