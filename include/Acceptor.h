#ifndef __ACCEPTER_H__
#define __ACCEPTER_H__

#include"utils.h"
class event_loop;
class channel;
class TCP_Server;
typedef function<void(int,sockaddr_in)> accept_cb; 
class Acceptor:noncopyable
{
    public:
        Acceptor(string ip,int port, event_loop* loop,int max_size);
        void listen();
        void set_connect_cb(accept_cb cb);
    private:
        void handleread();
        bool listerning;
        int fd;
        int max_size;
        shared_ptr<sockaddr_in> addr;
        shared_ptr<channel> channel_;
        event_loop* loop;
        accept_cb on_connect;
      
};
#endif // __ACCEPTER_H__