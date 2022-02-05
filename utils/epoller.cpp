#include "epoller.h"
#include "channel.h"
#include "event_loop.h"

epoller::epoller(event_loop *a) : owner(a)
{
    epoll_fd = epoll_create(a->size());
};
void epoller::update(channel *a)
{
    int fd = a->get_fd();
    // int event = a->get_events();
    epoll_event tmp;
    tmp.data.fd=fd;
    tmp.events=a->get_events();
    if (cb_map.find(fd) == cb_map.end())
    {
        cb_map[fd] = a;
      
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &tmp);
        a->set_registered(true);
    }
    else
    {
        if (!a->is_disable())
        {
            
            if (a->has_registered())
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &tmp);
            else
            {
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &tmp);
                a->set_registered(true);
            }
        }
        else
        {
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
            a->set_registered(false);
        }
    }
}
void epoller::unregister(channel *a)
{
    int fd = a->get_fd();
    
     if (cb_map.find(fd) != cb_map.end())
    {
        cb_map.erase(fd);
    }
    if (a->has_registered())
    {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
        a->set_registered(false);
    }
}
void epoller::run(int time_, channellist *out)
{
    vector<epoll_event> events_run(cb_map.size());
    int num = epoll_wait(epoll_fd, &events_run[0], cb_map.size(), time_);
    for (int i = 0; i < num; i++)
    {   //cout<<events_run[i].data.fd<<endl;
         if(cb_map.find(events_run[i].data.fd)!=cb_map.end()){
        auto j = cb_map[events_run[i].data.fd];
        j->set_revent(events_run[i].events);
        out->push_back(j);
        }
    }
};