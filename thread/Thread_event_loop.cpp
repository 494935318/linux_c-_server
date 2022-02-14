#include "Thread_event_loop.h"
#include"event_loop.h"
Thread_event_loop::Thread_event_loop(int size) :c_l(size), size_(size), pool(new t_pool(size)), now_(0)
{
    for (int i = 0; i < size; i++)
        pool->append(this);
    c_l.wait();
};
event_loop *Thread_event_loop::getnext()
{
    now_=(now_+1)%size_;
    return loop_vector[now_].get();
};
void Thread_event_loop::process(){
    shared_ptr<event_loop> loop(new event_loop());
    {
        lock_guard tmp(mutex_);
        loop_vector.push_back(loop);
    }
    c_l.count_down();
    loop->run();
};