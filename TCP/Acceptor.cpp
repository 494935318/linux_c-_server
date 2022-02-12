#include "Acceptor.h"
#include "event_loop.h"
#include "channel.h"
Acceptor::Acceptor(string ip, int port, event_loop *loop, int max_size) : loop(loop), max_size(max_size)
{
    addr.reset(get_addr_ipv4(ip.data(), 3026));
    int sock_tmp = socket(AF_INET, SOCK_STREAM, 0);
    int tmp1 = 1;
    setsockopt(sock_tmp, SOL_SOCKET, SO_REUSEADDR, &tmp1, sizeof(tmp1));
    int ret = bind(sock_tmp, (sockaddr *)addr.get(), sizeof(*addr));
    assert(ret == 0);
    assert(sock_tmp >= 0);
    set_nonblock(sock_tmp);
    fd = sock_tmp;
    channel_.reset(new channel(loop, sock_tmp));
    channel_->set_event(0);
    channel_->set_read_cb(bind(&Acceptor::handleread, this));
};

void Acceptor::listen()
{
    ::listen(fd, max_size);
    channel_->enable_read();
};
void Acceptor::set_connect_cb(accept_cb cb)
{
    on_connect = cb;
};
void Acceptor::handleread()
{
    sockaddr_in a;
    socklen_t b;
    int in_fd = accept4(fd, (sockaddr *)&a, &b, SOCK_CLOEXEC | SOCK_NONBLOCK);
    if (in_fd == -1)
    {
        auto save_err = errno;
        switch (save_err)
        {
        case EAGAIN:
        case EINTR:
        case EMFILE:
        case ECONNABORTED:
            break;
        default:
            close(fd);
            pthread_exit(nullptr);
            break;
        }
    }
    on_connect(in_fd, a);
}
