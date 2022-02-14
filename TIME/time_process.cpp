#include "time_process.h"
int timeout_connect(const char* ip, int port, int time_sec,int tv_usec=0)
{
    int ret=0;
    struct  sockaddr_in *address;
    
    address=get_addr_ipv4(ip,port);
    int sockfd=socket(PF_INET,SOCK_STREAM,0);
    assert(sockfd>=0);
    timeval timeout;
    timeout.tv_sec=time_sec;
    timeout.tv_usec=tv_usec;
    ret=setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
    assert(ret!=-1);
    ret=connect(sockfd,(sockaddr*)address,sizeof(*address));
    delete address;
    if(ret==-1){
        if(errno==EINPROGRESS||errno==EAGAIN){
            return 0;
        }
        else {
            return -1;
        }
    }
    if(ret==0) 
    return sockfd;
    return -1;
    
}
