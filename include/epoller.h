#ifndef __EPOLLER_H__
#define __EPOLLER_H__

#include"utils.h"
#include<unordered_map>
class event_loop;
class channel;
typedef vector<channel*> channellist;
class epoller:noncopyable
{
public:
    epoller(event_loop*a);
    void update(channel *a);
    void unregister(channel*a);
    void run(int time_,channellist* out);
private:
    unordered_map<int,channel*> cb_map;
    int epoll_fd;
    event_loop *owner;
};

#endif // __EPOLLER_H__