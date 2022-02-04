#ifndef __UTILS_H__
#define __UTILS_H__


#include"all_include.h"

// sockaddr* get_tcp_address(char * ip,int port);
sockaddr_in* get_addr_ipv4(const char * ip,int port);
sockaddr_in6* get_addr_ipv6(const char * ip,int port);
void print_getsockname(int fd);
void print_getpeername(int fd);
uint32_t  addfd(int epollfd ,int fd, int ctl=1,bool enable_et=false,bool oneshot=false);
void removefd(int epollfd,int fd);
void reset_oneshot(int epollfd,int fd);
int set_nonblock(int in_fd);
void mode_fd(uint32_t ctl,int fd,int epollfd);
class noncopyable
{   public:
        noncopyable()=default;
    private:
    noncopyable(const noncopyable&)=delete;
    const noncopyable & operator =(const noncopyable&)=delete;
};
void add_sig(int sig, __sighandler_t hand);
void sig_handler(int sig,int fd);
#endif // __UTILS_H__