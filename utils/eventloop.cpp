#include "event_loop.h"
#include "epoller.h"
#include "channel.h"
#include <sys/types.h>
int event_loop::sig_pipefd[2] = {-1, -1};
event_loop::event_loop(int size) : size_(size)
{
    owner_thread = gettid();
    epoller_.reset(new epoller(this));
    event_run_pid = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (sig_pipefd[0] == -1)
        socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd);
}
void event_loop::update(weak_ptr<channel> a)
{
    if (is_in_loopthread())
    {
        epoller_->update(a);
        // cout<<"has_updated"<<endl;
    }
    else
    {
        // cout<<"update_in_loop"<<endl;
        runsooner(bind(&event_loop::update, this, a));
    }
}

void event_loop::unregister(weak_ptr<channel> a)
{
    epoller_->unregister(a);
}
void event_loop::my_sig_handler(int a)
{
    sig_handler(a, sig_pipefd[0]);
}
void event_loop::addsig(int sig, callback_fun cb)
{

    sig_handle[sig] = cb;
    add_sig(sig, my_sig_handler);
}

time_id event_loop::runafter(int timeusec, callback_fun cb)
{
    return timer.set_time_callback(timeusec, cb);
}
time_id event_loop::runafter(timeval time, callback_fun cb)
{
    return timer.set_time_callback(time, cb);
}
time_id event_loop::runevery(timeval time, callback_fun cb)
{
    return timer.set_time_run_every(time, cb);
}
void event_loop::runsooner(callback_fun cb)
{
    if (is_in_loopthread())
    {
        cb();
    }
    else
    {
        {
            lock_guard a(mutex_);
            call_loop.push_back(cb);
        }
        wakeup();
    }
}

void event_loop::wakeup()
{

    // cout<<send(event_run_pid,&i,sizeof(i),0)<<endl;
    eventfd_write(event_run_pid, 1);
}
void event_loop::run()
{
    time_channel.reset(new channel(this, timer.time_fd));
    time_channel->set_read_cb(bind(&timer_fd::run, &timer));
    time_channel->enable_read();
    signal_channel.reset(new channel(this, sig_pipefd[1]));
    signal_channel->set_read_cb(bind(&event_loop::run_sig, this));
    signal_channel->enable_read();
    event_channel.reset(new channel(this, event_run_pid));
    event_channel->set_read_cb(bind(&event_loop::run_event, this));
    event_channel->enable_read();
    while (is_run)
    {
        vector<weak_ptr<channel>> out;
        epoller_->run(-1, &out);
        for (int i = 0; i < out.size(); i++)
        {
            auto tmp = out[i].lock();
            if (tmp)
                tmp->handle_event();
        }
        int n_run_soon = call_loop.size();
        for (int i = 0; i < n_run_soon; i++)
        {
            lock_guard a(mutex_);
            auto j = call_loop.front();
            call_loop.pop_front();
            a.unlock();
            j();
        }
    }
}
void event_loop::stop()
{
    is_run = false;
}
int event_loop::size()
{
    return size_;
}

bool event_loop::is_in_loopthread()
{
    return owner_thread == current_thread_id();
}

void event_loop::run_sig()
{
    int sig = 0;
    read(sig_pipefd[1], &sig, sizeof(sig));
    if (sig_handle.find(sig) != sig_handle.end())
    {
        auto tmp = sig_handle[sig];
        tmp();
    }
}
void event_loop::run_event()
{
    int tmp;
    // read(event_run_pid,&tmp,sizeof(tmp));
    eventfd_t count;
    eventfd_read(event_run_pid, &count);
}