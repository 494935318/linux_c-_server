#include"utils.h"
#include"locker.h"
#include"event_loop.h"
#include"TCP_Connect.h"
int run1 (int argc,char *argv[]){
    
    const char *ip="127.0.0.1";
    openlog("run1",LOG_PID|LOG_CONS,LOG_USER);
    setlogmask(LOG_UPTO(LOG_DEBUG));
    syslog(LOG_DEBUG,"%s","test2");
    closelog();
    auto addr1=get_addr_ipv4(ip,2233);
    addr1->sin_family=AF_INET;
    auto addr2=get_addr_ipv4(ip,1235);
    addr2->sin_family=AF_INET;//地址必须指定协议
    int sock_tmp=socket(AF_INET,SOCK_STREAM,0);
    bind(sock_tmp,(sockaddr*)addr1,sizeof(*addr1));
    if(connect(sock_tmp,(sockaddr*)addr2,sizeof(*addr2))==-1){
        cout<<"connect error"<<endl;
        return 0;
    }
    // close(sock_tmp);
    send(sock_tmp,"abc\r\n",5,0);
    char a[100];
    char * b=a;
    memset(a,'\0',100);
    int i=recv(sock_tmp,a,99,0);
    cout<<i<<endl;
    cout<<b;
    cout<<strlen(b)<<endl;
    // print_getsockname(sock_tmp);
    // print_getpeername(sock_tmp);
    cout<<"ok"<<endl;
    
    return 0;
}
count_down_latch a(1);
void * call(void *){
    sleep(1);
    a.count_down();
    return nullptr;
}
void test(int i){
    cout<<i<<endl;
}
void  run2(){
    pthread_t t;
    pthread_create(&t,NULL,call,NULL);
    auto c=bind(&test,1);
    c();
    cout<<time(NULL)<<endl;
    a.wait();
    cout<<time(NULL)<<endl;
    // return 0;
}
void run(event_loop&a){
    static int i=1;
    i++;
    cout<<i<<endl;
    timeval b;
    b.tv_sec=i;
    b.tv_usec=0;
    a.runafter(b,bind(run,ref(a)));
}
void run3(){
    static int i=1;
    i++;
    cout<<"the "<<i<<"th"<<endl;
}
int main(){
event_loop a;
timeval b;
const char *ip="127.0.0.1";
auto addr1=get_addr_ipv4(ip,3026);
int sock_tmp=socket(AF_INET,SOCK_STREAM,0);
int tmp=connect(sock_tmp,(sockaddr*)addr1,sizeof(*addr1));

 if(tmp!=0){
        cout<<"connect error"<<endl;
        return 0;
    }
string out="123\n\r";
 int n=send(sock_tmp,&out[0],5,0);
 cout<<n<<endl;
shared_ptr<TCP_Connect> tcp_client(new TCP_Connect( a,sock_tmp));
tcp_client->work();
tcp_client.reset();
a.run();
}

