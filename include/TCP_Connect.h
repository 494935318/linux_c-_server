#ifndef __TCP_CONNECT_H__
#define __TCP_CONNECT_H__

#include "utils.h"
#include <memory>
#include <netinet/tcp.h>
#include "buffer.h"
#include "locker.h"
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
    typedef function<void(shared_ptr<TCP_Connect>)> callback_fun_TCP;

public:
    TCP_Connect(int fd, event_loop *loop);
    void send(const string &in);
    void shurtdown();
    // void close();
    void forceclose();
    void work();
    ~TCP_Connect();
    void set_on_message(callback_fun_TCP);
    void set_on_close(callback_fun_TCP);
    void set_on_write_finish(callback_fun_TCP cb)
    {
        on_writefinish = cb;
    }
    void send_file(int fd, int size);
    int get_fd()
    {
        return fd;
    }
    void set_keep_alive(int a)
    {
        is_keepalive = true;

        setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &a, sizeof(a));
        if (a)
        {
            int keepidle = 2;     // 如该连接在60秒内没有任何数据往来,则进行探测
            int keepinterval = 1; // 探测时发包的时间间隔为5 秒
            int keepcount = 1;    // 探测尝试的次数。如果第1次探测包就收到响应了,则后2次的不再发。

            setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void *)&keepidle, sizeof(keepidle));
            setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval, sizeof(keepinterval));
            setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount, sizeof(keepcount));
        }
    };
    int get_owner_pid();
    Buffer::Buffer read_buf, write_buf;
    void set_context(const string &a, any b);
    any get_context(const string &a);

private:
    static void send_in_loop(weak_TCP in, const string &a);
    static void write(weak_TCP in);
    static void read(weak_TCP in);
    void close();

private:
    unordered_map<string, any> context;
    shared_ptr<TCP_Connect> holder;

private:
    bool is_keepalive = false;
    callback_fun_TCP on_message = 0;
    callback_fun_TCP on_close = 0;
    callback_fun_TCP on_writefinish = 0;
    bool onclose = false;
    bool iswriting = false;
    bool issendfile = false;
    int fd;
    event_loop *loop;
    shared_ptr<channel> channel_;
    list<shared_ptr<TCP_Connect>>::iterator idx;
};

#endif // __TCP_CONNECT_H__