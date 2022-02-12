#include "utils.h"
#include "config.h"
sockaddr *get_tcp_address(const char *ip, int port, bool ipv6 = false)
{
    decltype(AF_INET) af;
    if (ipv6)
        af = AF_INET6;
    else
        af = AF_INET;
    if (ipv6)
    {
        sockaddr_in6 *out = new sockaddr_in6();
        if (inet_pton(af, ip, (void *)&out->sin6_addr) == 0)
        {
            delete out;
            return nullptr;
        };
        out->sin6_port = htons(port);
        out->sin6_family=af;
        return (sockaddr *)out;
    }
    else
    {
        sockaddr_in *out = new sockaddr_in();
        if (inet_pton(af, ip, (void *)&out->sin_addr) == 0)
        {
            delete out;
            return nullptr;
        };
        out->sin_port = htons(port);
        out->sin_family=af;
        return (sockaddr *)out;
    }
}

sockaddr_in *get_addr_ipv4(const char *ip, int port)
{
    return (sockaddr_in *)get_tcp_address(ip, port, false);
}
sockaddr_in6 *get_addr_ipv6(const char *ip, int port)
{
    sockaddr *tmp = get_tcp_address(ip, port, true);
    return (sockaddr_in6 *)tmp;
}
void print_getsockname(int fd)
{
    sockaddr_in6 in6;
    unsigned int s = sizeof(in6);
    if (getsockname(fd, (sockaddr *)&in6, &s) == -1)
    {
        cout << "erron" << endl;
        return;
    };
    char a[100];
    memset(a, '\0', 100);
    if (s < sizeof(in6))
    {
        cout << inet_ntop(AF_INET, &((sockaddr_in *)&in6)->sin_addr, a, 100) << ":" << ntohs(((sockaddr_in *)&in6)->sin_port) << endl;
    }
    else
    {
        cout << inet_ntop(AF_INET6, &in6.sin6_addr, a, 100) << ":" << ntohs(in6.sin6_port) << endl;
    }
}
void print_getpeername(int fd)
{
    sockaddr_in6 in6;
    unsigned int s = sizeof(in6);
    if (getpeername(fd, (sockaddr *)&in6, &s) == -1)
    {
        cout << "erron" << endl;
        return;
    };
    char a[100];
    memset(a, '\0', 100);
    if (s < sizeof(in6))
    {
        cout << inet_ntop(AF_INET, &((sockaddr_in *)&in6)->sin_addr, a, 100) << ":" << ntohs(((sockaddr_in *)&in6)->sin_port) << endl;
    }
    else
    {
        cout << inet_ntop(AF_INET6, &in6.sin6_addr, a, 100) << ":" << ntohs(in6.sin6_port) << endl;
    }
}
// 判断是否为文件并返回文件大小,若为-1则文件无效
int isfile(const char *filename)
{
    struct stat file_stat;
    if (stat(filename, &file_stat) < 0)
    {
        return -1;
    }
    else
    {
        if (S_ISDIR(file_stat.st_mode))
        {
            return -1;
        }
        else if (file_stat.st_mode & S_IROTH)
        {
            return file_stat.st_size;
        }
        else
            return -1;
    }
}
bool set_keepalive(int in_fd, int value)
{
    // int value=1;
    return setsockopt(in_fd, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value)) == 0;
}
int set_nonblock(int in_fd)
{
    int old_option = fcntl(in_fd, F_GETFL);
    fcntl(in_fd, F_SETFD, old_option | O_NONBLOCK);
    return old_option;
}
int set_block(int in_fd){
     int old_option = fcntl(in_fd, F_GETFL);
    fcntl(in_fd, F_SETFD, old_option & ~O_NONBLOCK);
    return old_option;
}
uint32_t  addfd(int epollfd ,int fd, int ctl,bool enable_et,bool oneshot)
{
    epoll_event event;
    event.data.fd=fd;
    event.events=0;
    if(ctl&EVENT_IN)
    event.events=EPOLLIN;
    if(ctl&EVENT_OUT)
    {
    event.events|=EPOLLOUT;
    }
    if(enable_et){
        event.events|=EPOLLET;
    }
    if(oneshot){
        event.events|=EPOLLONESHOT;
    }
    else{
        event.events&=~EPOLLONESHOT;
    }
    uint32_t tmp_out= event.events;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    set_nonblock(fd);
    return tmp_out;
}
void mode_fd(uint32_t ctl,int fd,int epollfd){
 epoll_event event;
    event.data.fd=fd;
    event.events=ctl;
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);

}
void removefd(int epollfd,int fd){
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,NULL);
}
void reset_oneshot(int epollfd,int fd){
    epoll_event event;
    event.data.fd=fd;
    event.events=EPOLLIN|EPOLLONESHOT|EPOLLET;
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);
    set_nonblock(fd);

}
bool switch_to_user(uid_t user_id, gid_t gp_id)
{
    if (user_id == 0 && gp_id == 0)
    {
        return false;
    }
    gid_t gid = getgid();
    gid_t uid = getuid();
    // 确保当前用户是root或目标用户
    if ((gid != 0 || uid != 0) && (gid != gp_id || uid != user_id))
    {
        return false;
    }
    if (uid != 0)
    {
        return true;
    }
    if ((setgid(gp_id) < 0 || setuid(user_id) < 0))
    {
        return false;
    }
    syslog(LOG_DEBUG, "%s%d", "change uid 2 ", user_id);
    return true;
}
void sig_handler(int sig,int fd){
    int save_erron=errno;
    send(fd,&sig,sizeof(sig),0);
    errno=save_erron;
}
void add_sig(int sig, __sighandler_t hand){
struct  sigaction sa;
memset(&sa,'\0',sizeof(sa));
sa.sa_handler=hand;
sa.sa_flags|=SA_RESTART;
sigfillset(&sa.sa_mask);
assert(sigaction(sig,&sa,NULL)!=-1);
}

void send_fd(int fd, int fd_to_send){
    iovec iov[1];
    msghdr msg;
    char buf[0];
    iov[0].iov_base=buf;
    iov[0].iov_len=1;
    msg.msg_name=NULL;
    msg.msg_iov=iov;
    msg.msg_namelen=0;
    msg.msg_iovlen=1;
    cmsghdr cm;
    cm.cmsg_len=sizeof(int);
    cm.cmsg_level=SOL_SOCKET;
    cm.cmsg_type=SCM_RIGHTS;
    *(int *)CMSG_DATA(&cm)=fd_to_send;
    msg.msg_control=&cm;
    msg.msg_controllen=sizeof(int);
    sendmsg(fd,&msg,0);
}
int recv_fd(int fd){
    iovec iov[1];
    msghdr msg;
    char buf[0];
    iov[0].iov_base=buf;
    iov[0].iov_len=1;
    msg.msg_name=NULL;
    msg.msg_namelen=0;
    msg.msg_iov=iov;
    msg.msg_iovlen=1;
    cmsghdr cm;
    
    msg.msg_control=&cm;
    msg.msg_controllen=sizeof(int);
    recvmsg(fd,&msg,0);
    int fd_to_read=*(int*)CMSG_DATA(&cm);
    return fd_to_read;

}
int current_thread_id(){
    thread_local int a=-1;
    if(a==-1){
        a=gettid();
        // a=0;
    }
    return a;
}
std::string& trim(std::string &s) 
{
    if (!s.empty()) 
    {
        s.erase(0,s.find_first_not_of(" "));
    	s.erase(s.find_last_not_of(" ") + 1);
    }
    return s;
}
vector<string> split(string a, string b,int time){
    vector<string> out;
    int i=0,n=a.size();
    int used_time=0;
    while(i<n&&used_time<time){
        i=a.find_first_not_of(b,i);
        if(i==string::npos){
            return out;
        }
        int j=a.find_first_of(b,i);
        if(j!=string::npos){
        out.push_back(a.substr(i,j-i));
        }
        else{
         out.push_back(a.substr(i));
        return out;
        }
        i=j+1;
        used_time++;
    }
return out;
}
