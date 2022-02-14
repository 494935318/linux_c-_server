#include "utils.h"
#include <sys/timerfd.h>
#include <sys/time.h>
#include <utility>
#include "config.h"
#include <unordered_map>
#include<sys/eventfd.h>
#include"locker.h"
class TCP_Connect;
class event_loop;
typedef function<void()> callback_fun;
typedef pair<timeval, function<void()>>  tf_pair;
typedef shared_ptr<pair<timeval, function<void()>>> time_cb;
class channel;
class epoller;
struct time_cmp
{

    bool operator()(const time_cb &a, const time_cb &b)
    {
        if (a->first.tv_sec == b->first.tv_sec)
        {
            return a->first.tv_usec > b->first.tv_usec;
        }
        else
            return a->first.tv_sec > b->first.tv_sec;
    }
};
class time_id{
        public:
        time_id(time_cb b):a(b){

        }
           void stop(){
               auto tmp=a.lock();
               if(tmp){
                   tmp->second=0;
               }
           }
           bool finished(){
               auto tmp=a.lock();
               if(tmp) return false;
               return true;
           }
        private:
        weak_ptr<tf_pair> a;

};
class timer_fd
{
public:
    
    static timeval usec2timeval(int a)
    {
        timeval b;
        b.tv_usec = a % 1000000;
        b.tv_sec = a / 1000000;
        return b;
    }
    static int time_diff(timeval a, timeval b)
    {
        return (a.tv_sec - b.tv_sec) * 1000000 + a.tv_usec - b.tv_usec;
    }
    static timeval time_add(timeval a, int b)
    {
        a.tv_usec += b;
        a.tv_sec += a.tv_usec / 1000000;
        a.tv_usec %= 1000000;
        return a;
    }
    static timeval time_add(timeval a, timeval b)
    {
        a.tv_usec += b.tv_usec;
        a.tv_sec += a.tv_usec / 1000000 + b.tv_sec;
        a.tv_usec %= 1000000;
        return a;
    }

    timer_fd()
    {
        time_fd = timerfd_create(CLOCK_REALTIME, 0);
    }
    time_id set_time_callback(int t, function<void()> cb)
    {
        timeval now, next;
        gettimeofday(&now, NULL);
        next = time_add(now, t);
        lock_guard lock_tmp(time_lock);
        if (cb_queue.empty() || time_diff(next, cb_queue.top()->first) < 0)
        {
            setTimer(time_fd, -time_diff(now, next), 0);
        }
        auto tmp=time_cb(new tf_pair({next,cb}));
        cb_queue.push(tmp);
        return tmp;
    }
    time_id set_time_callback(timeval t, function<void()> cb)
    {
        timeval now, next;
        gettimeofday(&now, NULL);
        next = time_add(now, t);
        lock_guard lock_tmp(time_lock);
        if (cb_queue.empty() || time_diff(next, cb_queue.top()->first) < 0)
        {
            setTimer(time_fd, -time_diff(now, next), 0);
        }
        auto tmp=time_cb(new tf_pair({next,cb}));
        cb_queue.push(tmp);
        return tmp;
    }
    int set_absult_time_callback(int t, function<void()> cb)
    {
        return 0;
    }
    // t 微秒
    time_id set_time_run_every(int t, function<void()> cb)
    {
        timeval now, next;
        gettimeofday(&now, NULL);
        callback a(t, cb);
        next = time_add(now, t);
        lock_guard lock_tmp(time_lock);
        if (cb_queue.empty() || time_diff(next, cb_queue.top()->first) < 0)
        {
            setTimer(time_fd, -time_diff(now, next), 0);
        }
        auto tmp=time_cb(new tf_pair({next, bind(&timer_fd::run_every, this, a)}));
        cb_queue.push(tmp);
        return tmp;
    }
    time_id set_time_run_every(timeval t, function<void()> cb)
    {
        timeval now, next;
        gettimeofday(&now, NULL);
        callback a(t, cb);
        next = time_add(now, t);
        lock_guard lock_tmp(time_lock);
        if (cb_queue.empty() ||time_diff(next, cb_queue.top()->first) < 0)
        {
            setTimer(time_fd, -time_diff(now, next), 0);
        }
         auto tmp=time_cb(new tf_pair(next,cb));
         a.tmp=tmp;
         tmp->second=bind(&timer_fd::run_every, this, a);
        // auto tmp=time_cb(new tf_pair({next, bind(&timer_fd::run_every, this, a)}));
        cb_queue.push(tmp);
        return tmp;
    }
    void run()
    {
        uint64_t exp = 0;
        read(time_fd, &exp, sizeof(uint64_t));
        timeval now;
        gettimeofday(&now, NULL);
        while (!cb_queue.empty() && time_diff(now, cb_queue.top()->first) >= 0)
        {   lock_guard tmp1(time_lock);
            auto first = cb_queue.top();
            cb_queue.pop();
            tmp1.unlock();
            if(first->second)
            first->second();
            lock_guard tmp2(time_lock);
            if (time_diff(now, cb_queue.top()->first) < 0)
            {
                gettimeofday(&now, NULL);
            }
        }
        lock_guard tmp2(time_lock);
        if (!cb_queue.empty())
            setTimer(time_fd, -time_diff(now, cb_queue.top()->first), 0);
        else
        {
            setTimer(time_fd, 0, 0);
        }
    }
    friend class event_loop;

private:
    locker time_lock;
    void resettimer()
    {
    }
    class callback
    {
    public:
        callback(int t, function<void()> c) : wait_time(usec2timeval(t)), cb(c)
        {
        }
        callback(timeval t, function<void()> c) : wait_time(t), cb(c)
        {
        }
        timeval wait_time;
        function<void()> cb;
        time_cb tmp;
    };
    void run_every(callback cb)
    {
        cb.cb();
        timeval now, next;
        timeval time_wait = cb.wait_time;
        gettimeofday(&now, NULL);
        next = time_add(now, time_wait);
        lock_guard tmp2(time_lock);
        cb.tmp->first=next;
        cb_queue.push(cb.tmp);
    }

    priority_queue<time_cb, vector<time_cb>, time_cmp> cb_queue;
    void setTimer(int timerfd, int initialTime, int intervalTime)
    {
        struct itimerspec itimer;
        itimer.it_value.tv_sec = initialTime / 1000000;
        itimer.it_value.tv_nsec = initialTime % 1000000 * 1000;
        itimer.it_interval.tv_sec = intervalTime / 1000000;
        itimer.it_interval.tv_nsec = intervalTime % 1000000 * 1000;
        int ret = timerfd_settime(timerfd, 0, &itimer, NULL);
        if (-1 == ret)
        {
            perror("timerfd_settime");
        }
    }
    int time_fd;
};

class event_loop : noncopyable
{
public:
    event_loop(int size = 5)
    {
        epoll_fd = epoll_create(size);
        addfd(epoll_fd, timer.time_fd);
        if (sig_pipefd[0] == -1)
            socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd);
        addfd(epoll_fd, sig_pipefd[1]);
    }
    static void my_sig_handler(int a)
    {
        sig_handler(a, sig_pipefd[0]);
    }
    void addsig(int sig, callback_fun cb)
    {   
        lock_guard guard(epoll_lock);
        sig_handle[sig] = cb;
        add_sig(sig, my_sig_handler);
    }
    void add(int fd, callback_fun f, int ctl = 1, bool oneshot = false)
    {
        lock_guard guard(epoll_lock);
        if (fd_ctl.find(fd) == fd_ctl.end())
        {
            if (ctl & EVENT_IN)
            {
                in_handle[fd] = f;
            }
            if (ctl & EVENT_OUT)
            {
                out_handle[fd] = f;
            }
            auto tmp=addfd(epoll_fd, fd, ctl, true, oneshot);
            fd_ctl[fd] = tmp;
            event_num++;
        }
        else
        {
            uint32_t tmp = fd_ctl[fd];
            if (ctl & EVENT_IN)
            {
                in_handle[fd] = f;
                tmp |= EPOLLIN;
            }
            if (ctl & EVENT_OUT)
            {
                out_handle[fd] = f;
                tmp |= EPOLLOUT;
            }
            if(oneshot){
                tmp|=EPOLLONESHOT;
            }else{
                 tmp&=~EPOLLONESHOT;
            }
            mode_fd(tmp, fd, epoll_fd);
            fd_ctl[fd] = tmp;
        }
    }
    void reset_stat(int fd,bool oneshot=true){
        lock_guard guard(epoll_lock);
        auto tmp=fd_ctl[fd];
        if(oneshot){
            tmp|=EPOLLONESHOT;
        }
        else
        {
            tmp&=!EPOLLONESHOT;
        }
        mode_fd(fd_ctl[fd],fd,epoll_fd);
    }
    void remove(int fd, int ctl)
    {   
        lock_guard guard(epoll_lock);
        auto tmp = fd_ctl[fd];
        if ((ctl & EVENT_IN)&&(tmp&EPOLLIN))
        {   
            in_handle.erase(fd);
            tmp &= ~EPOLLIN;
        }
        if ((ctl & EVENT_OUT)&&(tmp&EPOLLOUT))
        {
            out_handle.erase(fd);
            tmp &= ~EPOLLOUT;
        }
        if (!(tmp & EPOLLIN) && !(tmp & EPOLLOUT))
        {
            fd_ctl.erase(fd);
            removefd(epoll_fd, fd);
            event_num--;
        }
        else
        {
            mode_fd(tmp, fd, epoll_fd);
            fd_ctl[fd] = tmp;
        }
    }
    
    void runafter(int timeusec, callback_fun cb)
    {
        timer.set_time_callback(timeusec, cb);
    }
    void runafter(timeval time, callback_fun cb)
    {
        timer.set_time_callback(time, cb);
    }
    void runevery(timeval time, callback_fun cb)
    {
        timer.set_time_run_every(time, cb);
    }
    void runsooner(callback_fun cb)
    {   lock_guard a(epoll_lock);
        call_loop.push_back(cb);
    }

    void run()
    {
        while (is_run)
        {
            epoll_event *event = new epoll_event[event_num];
            int n = epoll_wait(epoll_fd, event, event_num, 100000);
            for (int i = 0; i < n; i++)
            {
                int fd = event[i].data.fd;
                if (fd == timer.time_fd)
                {
                    timer.run();
                }
                else if (fd == sig_pipefd[1])
                {
                    int sig = 0;
                    read(sig_pipefd[1], &sig, sizeof(sig));
                    lock_guard guard(epoll_lock); 
                    if (sig_handle.find(sig) != sig_handle.end())
                    {
                        auto tmp=sig_handle[sig];
                        guard.unlock();
                        tmp();
                    }
                }
                else
                {
                    if (event[i].events & EPOLLIN)
                    {    lock_guard guard(epoll_lock); 
                        if (in_handle.find(fd) != in_handle.end())
                        {   
                            auto tmp= in_handle[fd];
                            guard.unlock();
                            tmp();
                           
                        }
                    }
                    if (event[i].events & EPOLLOUT)
                    {   
                      lock_guard guard(epoll_lock); 

                        if (out_handle.find(fd) != out_handle.end())
                        {   auto tmp= out_handle[fd];
                            guard.unlock();
                            tmp();
                        }
                    }
                }
            }
            delete[] event;
            int n_run_soon = call_loop.size();
            for (int i = 0; i < n_run_soon; i++)
            {   auto j = call_loop.front();
                j();
                call_loop.pop_front();
            }
        }
    }
    void stop()
    {
        is_run = false;
    }
    
private:
    void addwrite(int fd, callback_fun f)
    {
        epoll_event event;
        event.data.fd = fd;
        event.events = EPOLLOUT;
        event.events |= EPOLLET;
        event.events |= EPOLLONESHOT;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
        set_nonblock(fd);
        out_handle[fd] = f;
    }
    int epoll_fd;
    locker epoll_lock;
    unordered_map<int, uint32_t> fd_ctl;
    unordered_map<int, callback_fun> in_handle;
    unordered_map<int, callback_fun> out_handle;
    unordered_map<int, callback_fun> sig_handle;
    static int sig_pipefd[2];

    bool is_run = true;
    int event_num = 10;
    timer_fd timer;
    list<callback_fun> call_loop;
};
int event_loop::sig_pipefd[2] = {-1, -1};
