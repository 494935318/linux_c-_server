#ifndef __TCP_CONNECT_OLD_H__
#define __TCP_CONNECT_OLD_H__

#include "utils.h"
#include "event_loop_old.h"
#include "buffer.h"
#include "locker.h"
class TCP_Connect;
locker list_lock;
list<shared_ptr<TCP_Connect>> mem_TCP_list;
void send_in_loop(weak_ptr<TCP_Connect> con, string a);
void send_all_in_loop(weak_ptr<TCP_Connect> con, bool wait_close = false);
void default_on_message(weak_ptr<TCP_Connect> con);
class TCP_Connect : noncopyable, public enable_shared_from_this<TCP_Connect>
{
    typedef function<void(weak_ptr<TCP_Connect>)> func;
    typedef shared_ptr<TCP_Connect> TCP_ConnectPtr;
    friend void send_in_loop(weak_ptr<TCP_Connect> con, string a);
    friend void send_all_in_loop(weak_ptr<TCP_Connect> con, bool wait_close);

public:
    TCP_Connect(event_loop &loop_, int fd, func on_messge_=default_on_message) : loop(loop_), on_messge(on_messge_), fd(fd){

                                                                                                  };
    void send(string a)
    {   if(can_write)
        loop.runsooner(bind(send_in_loop, shared_from_this(), a));
    }
    static void  read(weak_ptr<TCP_Connect> con)
    {
        auto con_sh=con.lock();
        if(con_sh){
           int num= con_sh->readbuff.read_fd(con_sh->fd);
           if (num==0)
           {
               close(con_sh);
           }
           else{
               con_sh->on_messge(con);
           }
        }


    }
    // friend class TCP_ConnectPtr;
    
    void work()
    {
        lock_guard tmp(list_lock);
        mem_TCP_list.push_front(shared_from_this());
        index = mem_TCP_list.begin();
        loop.add(fd, bind(read, shared_from_this()), EVENT_IN);
    }
    // 得到对面关闭信号后的关闭,将所有数据发送后关闭
    static void close(weak_ptr<TCP_Connect> con)
    {
        auto con_shr = con.lock();
        if (con_shr)
        { 
            // 禁止继续写入
              con_shr->can_write=false;
            // 删除读等待
            con_shr->loop.remove(con_shr->fd,EVENT_IN);
            // 判断是否还可写
            if (con_shr->writebuff.is_empty())
            {
                // 执行关闭
                con_shr->close();
            }
            else
            {
                //  进行最后的写
                con_shr->loop.add(con_shr->fd,bind(send_all_in_loop, con_shr,true),EVENT_OUT);
            }
        }
    }
    static void shutdowninloop(weak_ptr<TCP_Connect> con){
         auto con_shr = con.lock();
        if (con_shr)
        {
            // 发送队列为空,关闭发送端口
            if (con_shr->writebuff.is_empty())
            {
                shutdown(con_shr->fd,SHUT_WR);
            }
            else
            // 发送数据重新判断
            {   con_shr->writebuff.sendfd(con_shr->fd);
                con_shr->loop.add(con_shr->fd,bind(shutdowninloop, con_shr),EVENT_OUT);
            }
        }
    }
    // 关闭写端口
    // 等待对面的关闭信号
    void termit(){
        // 禁止写
        can_write=false;
        // 判断并关闭写口
        loop.runsooner(bind(shutdowninloop,shared_from_this()));
    }
    ~TCP_Connect()
    {
        loop.remove(fd, 3);
        cout<<"closed"<<endl;
        ::close(fd);
    }
Buffer::Buffer writebuff;
    Buffer::Buffer readbuff;
    
private:
    // 关闭连接,执行析构
    void close()
    {
        lock_guard tmp(list_lock);
        mem_TCP_list.erase(index);
    }
    func on_messge;
    int _is_long = false;
    bool can_write=true;
    const int fd;
    int epoll_fd;
    event_loop &loop;
    list<shared_ptr<TCP_Connect>>::iterator index;
};
void send_all_in_loop(weak_ptr<TCP_Connect> con, bool wait_close)
{
    auto con_shr = con.lock();
    if (con_shr)
    {
        int remain = con_shr->writebuff.sendfd(con_shr->fd);
        if (remain == 0)
        {
            con_shr->loop.remove(con_shr->fd, EVENT_OUT);
            // 判断是否需要关闭
            if (wait_close)
            {
                con_shr->close();
            }
        }
    }
}
void send_in_loop(weak_ptr<TCP_Connect> con, string a)
{
    auto con_shr = con.lock();
    if (con_shr)
    {
        con_shr->writebuff.append(a);
        if(con_shr->can_write)
        con_shr->loop.add(con_shr->fd, bind(send_all_in_loop, con_shr,false), EVENT_OUT);
    }
}
void default_on_message(weak_ptr<TCP_Connect> con){
    auto con_sh=con.lock();
    if(con_sh){
        string a(con_sh->readbuff.begin(),con_sh->readbuff.end());
        cout<<a<<endl;
        con_sh->readbuff.retrieve(con_sh->readbuff.readable_size());
        con_sh->send(a);
    }


}
#endif // __TCP_CONNECT_OLD_H__