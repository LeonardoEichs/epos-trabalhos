// EPOS Thread Abstraction Declarations

#ifndef __idle_thread_h
#define __idle_thread_h

#include <thread.h>

extern "C" { void __exit(); }

__BEGIN_SYS

class IdleThread : public Thread {
private:
    static const unsigned int priority = Thread::IDLE;
    static const Thread::State state = Thread::READY;
    static int do_nothing() {
        while (true) {
            CPU::int_enable();
            CPU::halt();
        }
        return 0;
    }

public:

    IdleThread() :
    Thread(Thread::Configuration(state, priority), &do_nothing) {}
};

__END_SYS

#endif
