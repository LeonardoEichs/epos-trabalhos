// EPOS Alarm Abstraction Implementation

#include <semaphore.h>
#include <alarm.h>
#include <display.h>

#define MAX_ELAPSED 1000000
__BEGIN_SYS

// Class attributes
Alarm_Timer * Alarm::_timer;
volatile Alarm::Tick Alarm::_elapsed;
Alarm::Queue Alarm::_request;


// Methods
Alarm::Alarm(const Microsecond & time, Handler * handler, int times)
: _ticks(ticks(time)), _handler(handler), _times(times), _link(this, _ticks)
{
    lock();

    db<Alarm>(TRC) << "Alarm(t=" << time << ",tk=" << _ticks << ",h=" << reinterpret_cast<void *>(handler)
                   << ",x=" << times << ") => " << this << endl;

    if(_ticks) {
        _request.insert(&_link);
        unlock();
    } else {
        unlock();
        (*handler)();
    }
}


Alarm::~Alarm()
{
    lock();

    db<Alarm>(TRC) << "~Alarm(this=" << this << ")" << endl;

    _request.remove(this);

    unlock();
}


// Class methods
void Alarm::delay(const Microsecond & time)
{
    db<Alarm>(TRC) << "Alarm::delay(time=" << time << ")" << endl;
    Thread_Handler * th = new Thread_Handler(&blank);
    Alarm * a = new Alarm(time, th);
    th->_joining = this;
    //Tick t = _elapsed + ticks(time);

    //while(_elapsed < t);
}


void Alarm::handler() {
  static Tick next_tick;
  static Handler * next_handler;
  static Tick runned;

  lock();
  _elapsed++;
  if (runned++ > MAX_ELAPSED && next_handler) {
    if (!_request.empty()) {
      int r = _request.tail()->rank();
      Alarm * old = new Alarm((r+1)*1000000, next_handler);
      Queue::Element *ne = _request.remove();
      Alarm * alarm = ne->object();
      next_tick = alarm->_ticks;
      next_handler = alarm->_handler;
      if (alarm->_times != -1) alarm->_times--;
      if (alarm->_times) {
        ne->rank(alarm->_ticks);
        _request.insert(ne);
      }
    }
  runned = 0;
  }
  if (next_tick) next_tick--;
  if (!next_tick) {
    if (next_handler) {
      db<Alarm>(TRC) << "Alarm::handler(h=" << reinterpret_cast<void*>(next_handler) << ")" << endl;
      unlock();
      next_handler->run();
      lock();
    }
    if(_request.empty()) next_handler = 0;
    else {
      Queue::Element * e = _request.remove();
      Alarm * alarm = e->object();
      next_tick = alarm->_ticks;
      next_handler = alarm->_handler;
      if(alarm->_times != -1) alarm->_times--;
      if(alarm->_times) {
        e->rank(alarm->_ticks);
        _request.insert(e);
      }
      runned=0;
    }
  }
  unlock();
}

__END_SYS
