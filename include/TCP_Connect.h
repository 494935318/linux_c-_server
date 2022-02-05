#ifndef __TCP_CONNECT_H__
#define __TCP_CONNECT_H__

#include "utils.h"
#include<memory>
#include "buffer.h"
#include"locker.h"
class TCP_Connect;
class event_loop;
class channel;
typedef weak_ptr<TCP_Connect> weak_TCP;
typedef function<void()> callback_fun;
// typedef function<void(weak_ptr<TCP_Connect>)> func;
void default_on_message(weak_ptr<TCP_Connect> con);
void shut_down_inloop(weak_TCP in);
class TCP_Connect : noncopyable, public enable_shared_from_this<TCP_Connect>
{
    typedef function<void (shared_ptr<TCP_Connect>)> callback_fun_TCP;
   public:
   TCP_Connect(int fd, event_loop* loop);
    void send(const string &in);
    void shurtdown();
    // void close();
    void forceclose();
    void work();
    ~TCP_Connect();
    void set_on_message(callback_fun_TCP);
    void set_on_close(callback_fun_TCP);
    void set_on_write_finish(callback_fun_TCP cb){
        on_writefinish=cb;
    }
    int get_fd(){
        return fd;
    }
    int get_owner_pid();
    Buffer::Buffer read_buf,write_buf;
   private:

   static void send_in_loop(weak_TCP in,string a);
    static void write(weak_TCP in);
    static void read(weak_TCP in);
   void close();
   callback_fun_TCP on_message=0;
   callback_fun_TCP on_close=0;
   callback_fun_TCP on_writefinish=0;
    bool onclose=false;
    bool iswriting=false;
    int fd;
    event_loop * loop;
    shared_ptr<channel> channel_;
    list<shared_ptr<TCP_Connect>>::iterator idx;
};

#endif // __TCP_CONNECT_H__