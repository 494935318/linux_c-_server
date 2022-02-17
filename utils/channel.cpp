#include "channel.h"
#include "event_loop.h"
channel::channel(event_loop *loop, int fd) : loop(loop), fd(fd)
{
    event_ = EPOLLET;
};

void channel::disableall()
{
    disabled = true;
    update();
}
void channel::restart()
{
    disabled = false;
    update();
}

void channel::enable_write()
{
    disabled = false;
    writeable = true;
    event_ |= EPOLLOUT;
    update();
}
void channel::enable_read()
{
    readable = true;
    disabled = false;
    event_ |= EPOLLIN;
    update();
}
void channel::disable_write()
{
    writeable = false;
    event_ &= ~EPOLLOUT;
    update();
}
void channel::disable_read()
{
    readable = false;
    event_ &= ~EPOLLIN;
    update();
}
void channel::set_error_cb(const callback_fun &f)
{
    erro_cb = f;
}
void channel::set_read_cb(const callback_fun & f)
{
    read_cb = f;
}
void channel::set_write_cb(const callback_fun & f)
{
    write_cb = f;
}
void channel::update()
{

    loop->update(weak_ptr<channel>(shared_from_this()));
}
void channel::handle_event()
{
    event_handling = true;
    if (r_event & EPOLLIN)
    {
        if (read_cb)
            read_cb();
    }
    if (r_event & EPOLLOUT)
    {
        if (write_cb)
            write_cb();
    }
    if (r_event & EPOLLERR)
    {
        if (erro_cb)
            erro_cb();
    }
    event_handling = false;
}
int channel::get_fd()
{
    return fd;
}
uint32_t channel::get_events()
{
    return event_;
}
void channel::set_event(uint32_t a){
    event_=a;
};
bool channel::is_disable()
{
    return disabled;
}
bool channel::has_registered()
{
    return registered;
}
void channel::set_registered(bool a)
{
    registered = a;
}
void channel::set_revent(uint32_t r)
{
    r_event = r;
}