#ifndef PROCESS_POLL
#define PROCESS_POLL
#include"utils.h"

class process{
    public:
    process():m_pid(-1){}
    //进程号
    pid_t m_pid;
    //管道
    int m_pipefd[2];
};
template<class T>
class process_pool:noncopyable{
    private:
    process_pool(int listenfd,int process_number=8);
    public:
    static process_pool * creat(int listenfd,int process_number=8)
    {
        if(!m_instance){
            m_instance=new process_pool(listenfd,process_number);
        }
        return m_instance;
    }
    void run();
    private:
    void setup_sig_pipe();
    void run_parent();
    void run_child();
    static const int MAX_PROCESS_NUMBER=16;
    static const int USER_PER_PROCESS=65536;
    static const int MAX_EVENT_NUMBER=10000;
    int m_process_number;
    int m_idx;
    int m_epollfd;
    int m_listenfd;
    int m_stop;
    process * m_sub_process;
    static process_pool<T>* m_instance;
};
template<class T>
process_pool<T>* process_pool<T>:: m_instance=NULL;
template<class T> 
process_pool<T>::process_pool(int listenfd,int process_number):m_listenfd(listenfd),m_process_number(process_number),m_idx(-1),m_stop(false)
{
assert(process_number>0&& process_number<=MAX_PROCESS_NUMBER) ;

m_sub_process=new process[process_number];
assert(m_sub_process);
for(int i=0;i<process_number;i++){
    int ret=socketpair(PF_UNIX,SOCK_STREAM,0,m_sub_process[i].m_pipefd);
    assert(ret==0);
    m_sub_process[i].m_pid=fork();
    assert(m_sub_process[i].m_pid>=0);
    if(m_sub_process[i].m_pid==0){
        close(m_sub_process[i].m_pipefd[0]);
        m_idx=i;
        break;
    }else{
        close(m_sub_process[i].m_pipefd[1]);
        continue;
    }
}
}
template<class T> 
void process_pool<T>::run(){
    if(m_idx==0) run_parent();
    else run_child();
}
template<class T> 
void process_pool<T>::run_child(){
    T* user=new T(m_sub_process[m_idx],m_listenfd);
    user->run_child();
    delete(user);
}
template<class T> 
void process_pool<T>::run_parent(){
    T* user=new T(this);
    user->run_parent();
    delete(user);
}
#endif
