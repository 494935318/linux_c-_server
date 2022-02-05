#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include"utils.h"
// #include"event_loop.h"
// 事件分发器
class event_loop;
typedef function<void()> callback_fun;
class channel:noncopyable
{
    public:
    channel(event_loop *loop,int fd);
    void restart();
    void disableall();
    void enable_write();
    void enable_read();
    void disable_write();
    void disable_read();
    void set_error_cb(callback_fun f);
    void set_read_cb(callback_fun f);
    void set_write_cb(callback_fun f);
    void update();
    void handle_event();
    int get_fd();
    uint32_t get_events();
    void set_event(uint32_t a);
    bool is_disable();
    bool has_registered();
    void set_registered(bool a);
    void set_revent(uint32_t r);
    private:
    bool event_handling=false;
    uint32_t event_;
    uint32_t r_event;
    bool writeable=false;
    bool readable=false;
    bool disabled=false;
    bool registered=false;
    int fd;
    callback_fun read_cb=0,write_cb=0,erro_cb=0;
    event_loop* loop;
};
#endif // __CHANNEL_H__