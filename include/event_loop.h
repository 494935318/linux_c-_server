#include"utils.h"
 #include <sys/timerfd.h>
 #include<sys/time.h>
 #include<utility>
  
class TCP_Connect;
class event_loop;
typedef function<void()>callback_fun;
class timer_fd:noncopyable
{
    public:
        static int time_diff(timeval a, timeval b)
        {
            return (a.tv_sec-b.tv_sec)*1000000+a.tv_usec-b.tv_usec;
        }
        static timeval time_add(timeval a,int b){
            a.tv_usec+=b;
            a.tv_sec+=a.tv_usec/1000000;
            a.tv_usec%=1000000;
            return a;
        }

        timer_fd():cb_queue(time_cmp){
            time_fd=timerfd_create(CLOCK_REALTIME,0);
        }
        int set_time_callback(int t,function<void()> cb){
             timeval now,next;
            gettimeofday(&now,NULL);
            next=time_add(now,t);
            if(time_diff(next,cb_queue.top().first)<0){
            setTimer(time_fd,-time_diff(now,next),0);
            }
            cb_queue.push({next,cb});   
        }
         int set_absult_time_callback(int t,function<void()> cb){
            
        }
        // t 微秒
        int set_time_run_every(int t,function<void()>cb){
            timeval now,next;
            gettimeofday(&now,NULL);
            callback a(t,cb);
            next=time_add(now,t);
            if(time_diff(next,cb_queue.top().first)<0){
            setTimer(time_fd,-time_diff(now,next),0);
            }
            cb_queue.push({next,bind(&run_every,this,a)});   
        }
       void run(){
            timeval now;
            gettimeofday(&now,NULL);
            while(!cb_queue.empty()&& time_diff(now,cb_queue.top().first)>=0 )
            {
                auto first=cb_queue.top();
                cb_queue.pop();
                first.second();
                if(time_diff(now,cb_queue.top().first)<0){
                     gettimeofday(&now,NULL);
                }
            }
            if(!cb_queue.empty())
            setTimer(time_fd,-time_diff(now,cb_queue.top().first),0);
            else{
                setTimer(time_fd,0,0);
            }
        }
    friend class event_loop;
    private:
        void resettimer(){
            
        }
        class callback{
            public:
                callback(int t, function<void()> c):wait_time(t),cb(c)
                {

                }
                int wait_time;
                function<void()>cb;
        };
        void run_every(callback cb){
            cb.cb();
            timeval now,next;
            int time_wait=cb.wait_time;
            gettimeofday(&now,NULL);
            next=time_add(now,time_wait);
            cb_queue.push({next,bind(&run_every,this,cb)});
            
        }
    typedef pair<timeval,function<void()>> time_cb;
    static bool time_cmp(time_cb a, time_cb b){
        if(a.first.tv_sec==b.first.tv_sec){
            return a.first.tv_usec>b.first.tv_usec;

        }
        else 
        return a.first.tv_sec>b.first.tv_sec;
    }
    
    priority_queue<time_cb,vector<time_cb>,decltype(time_cmp)> cb_queue;
    void setTimer(int timerfd, int initialTime, int intervalTime)
    {
    struct itimerspec itimer;
    itimer.it_value.tv_sec = initialTime;
    itimer.it_value.tv_nsec = 0;
    itimer.it_interval.tv_sec = intervalTime;
    itimer.it_interval.tv_nsec = 0;
    int ret = timerfd_settime(timerfd, 0, &itimer, NULL);
    if (-1 == ret)
    {
        perror("timerfd_settime");
    }
    }
  int time_fd;
};
class event_loop:noncopyable
{
    public: 
    event_loop(int size=5){
        epoll_fd=epoll_create(size);
        add(timer.time_fd,bind(&timer_fd::run,timer));
    }
    void add(int fd ,callback_fun f){
        addfd(epoll_fd,fd,false,false);
    }
    void remove(int fd){
        removefd(epoll_fd,fd);
    }
    void runafter(int timeusec,callback_fun cb){
        timer.set_time_callback(timeusec,cb);
    }
    friend class TCP_Connect;
    private:
    timer_fd timer;
    list<function<void()>> call_loop;
    int  epoll_fd;
    unordered_map<int,callback_fun> call_back_handle;
    
};