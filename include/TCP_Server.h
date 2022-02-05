#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include"utils.h"
class event_loop;
class Acceptor;
class TCP_Connect;
class Thread_event_loop;
typedef function<void(shared_ptr<TCP_Connect>)> server_callback_fun;
class TCP_Server:noncopyable{
public:
    TCP_Server(event_loop* ,string ip,int port ,int size=2000);
    void set_on_connect(server_callback_fun );
    void set_on_message(server_callback_fun);
    void work();
    void set_threadnum(int size);
private:
    void remove_connect(int fd);
    void on_new_connect(int ,sockaddr_in );
    void on_connect_close(shared_ptr<TCP_Connect>);
private:  
    bool multi_thread=false;
    event_loop * loop;
    shared_ptr<Thread_event_loop> loop_pool;
    unordered_map<int,weak_ptr<TCP_Connect>> connect_map;
    shared_ptr<Acceptor> acceptor_;
    server_callback_fun on_connect=0;
    server_callback_fun on_message=0;


};
#endif // __TCP_SERVER_H__