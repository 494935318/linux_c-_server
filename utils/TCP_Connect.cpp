#include "TCP_Connect.h"
#include "channel.h"
#include "event_loop.h"
locker TCP_list_lock;
list<shared_ptr<TCP_Connect>> mem_TCP_list;
TCP_Connect::TCP_Connect(int fd, event_loop *loop) : fd(fd), loop(loop)
{
    channel_.reset(new channel(loop, fd));
    on_message = default_on_message;
};
void TCP_Connect::work()
{
    //  int n1=shared_from_this().use_count();
    {
        lock_guard tmp(TCP_list_lock);
        mem_TCP_list.push_front(shared_from_this());
        idx = mem_TCP_list.begin();
    }
    channel_->set_read_cb(bind(&TCP_Connect::read, weak_TCP(shared_from_this())));
    channel_->set_write_cb(bind(&TCP_Connect::write, weak_TCP(shared_from_this())));
    channel_->enable_read();
    //  n1=shared_from_this().use_count();
}
void TCP_Connect::send(const string &in)
{
    // 关闭后发送直接无效
    if (onclose)
        return;

    if (loop->is_in_loopthread())
    {
        //     if(!iswriting){
        //     iswriting=true;
        //     channel_->enable_write();
        // }
        if (iswriting)
        {
            write_buf.append(in);
        }
        else
        {
            int n = ::send(fd, &in[0], in.size(), 0);

            if (n < 0)
            {
                close();
            }
            else if (n == 0)
            {
                if (errno != EAGAIN)
                    close();
            }
            else if (n < in.size())
            {
                write_buf.append(&in[n], in.size() - n);
                iswriting = true;
                channel_->enable_write();
            }
            if (on_writefinish)
                on_writefinish(shared_from_this());
        }
    }
    else
    {
        loop->runsooner(bind(&TCP_Connect::send_in_loop, weak_TCP(shared_from_this()), in));
    }
};
// 在事件循环内进行
void TCP_Connect::send_in_loop(weak_TCP in, string a)
{
    auto tmp = in.lock();
    if (tmp)
    {
        tmp->send(a);
    }
};
void TCP_Connect::write(weak_TCP in)
{
    auto tmp = in.lock();
    if (tmp)
    {
        int n = tmp->write_buf.sendfd(tmp->fd);
        if (tmp->write_buf.readable_size() == 0)
        {
            tmp->channel_->disable_write();
            tmp->iswriting = false;
            if (tmp->on_writefinish)
                tmp->on_writefinish(tmp);
            if (tmp->onclose)
            {
                shutdown(tmp->fd, SHUT_WR);
            }
        }
        if (n < 0)
        {
            tmp->close();
        }
    }
};
void TCP_Connect::read(weak_TCP in)
{
    auto tmp = in.lock();
    if (tmp)
    {
        int tmp_n = tmp.use_count();
        int n = tmp->read_buf.read_fd(tmp->fd);
        if (n <= 0)
        {
            tmp->close();
        }
        else
        {
            if (tmp->on_message)
                tmp->on_message(tmp);
        }
    }
};
void TCP_Connect::shurtdown()
{
    if (loop->is_in_loopthread())
    {
        if (iswriting)
        {
            onclose = true;
        }
        else
        {
            shutdown(fd, SHUT_WR);
            channel_->disable_write();
            onclose = true;
        }
    }
    else
    {
        loop->runsooner(bind(shut_down_inloop, weak_TCP(shared_from_this())));
    }
};
// 将关闭写端口加入事件循环
void shut_down_inloop(weak_TCP in)
{
    auto tmp = in.lock();
    if (tmp)
    {
        tmp->shurtdown();
    }
}
// 强制关闭_in
void TCP_Connect::close()
{
    onclose = true;
    loop->unregister(channel_.get());
    int tmp1 = channel_.use_count();
    // int n=shared_from_this().use_count();
    //  n=shared_from_this().use_count();
    if (on_close)
        on_close(shared_from_this());
    lock_guard tmp(TCP_list_lock);
    mem_TCP_list.erase(idx);
};
void TCP_Connect::forceclose()
{
    close();
};
TCP_Connect::~TCP_Connect()
{
    //  loop->unregister(channel_.get());
    ::close(fd);
    cout << "closed" << endl;
};

void TCP_Connect::set_on_message(callback_fun_TCP cb)
{
    on_message = cb;
};
void TCP_Connect::set_on_close(callback_fun_TCP cb)
{
    on_close = cb;
};
int TCP_Connect::get_owner_pid()
{
    return loop->get_owner_pid();
};
void default_on_message(weak_ptr<TCP_Connect> con)
{
    auto con_sh = con.lock();
    if (con_sh)
    {
        string a(con_sh->read_buf.begin(), con_sh->read_buf.end());
        cout << a << endl;
        con_sh->read_buf.retrieve(con_sh->read_buf.readable_size());
        con_sh->send(a);
    }
}
