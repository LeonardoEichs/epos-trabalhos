// EPOS Handler Utility Declarations

#ifndef __handler_h
#define __handler_h

#include <system/config.h>
#include <thread.h>

__BEGIN_SYS

class Handler {
public:
  Handler(){}
  ~Handler(){}
  virtual void run() = 0;

}

class Function_Handler : public Handler {
public:
  Function_Handler(void (* entry)()) {
    func = &entry;
  }

  ~Function_Handler() {};
  void run() { func(); }
private:
  void (* func)();
}

class Thread_Handler : public Handler {
public:
  Thread_Handler(void (* entry)()) {
    _thread = new Thread(&entry, WAITING, HIGH);
  }
  ~Thread_Handler() {delete _thread; }
  void run() {
    _thread->_state = READY;
    Thread::insert(&_thread->link);
    Thread::yield();
  }
private:
  Thread * _thread;
}

__END_SYS

#endif
