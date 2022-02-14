#include"utils.h"
#include"thread_pool.h"
class event_loop;
class Thread_event_loop;
typedef thread_pool<Thread_event_loop> t_pool;
class Thread_event_loop:noncopyable
{
    public: 
    Thread_event_loop(int size);
    event_loop* getnext();
    void process();
    private:
    int size_;
    int now_;
    locker mutex_;
    shared_ptr<t_pool> pool;
    vector<shared_ptr<event_loop>> loop_vector;
    count_down_latch c_l;
};