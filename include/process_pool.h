#ifndef PROCESS_POLL
#define PROCESS_POLL
#include"utils.h"
class process{
    public:
    process():m_pid(-1){}
    private:
    //进程号
    pid_t m_pid;
    //管道
    int m_pipefd[2];
};
template<class T>
class process_pool{
    private:
    process_pool(int listenfd,int process_number=8);
    public:
    static process_pool * creat(int listenfd,int process_number=8)
    {
        if(!m_instance){
            m_instance=new process_pool(listenfd,process_number);
        }
        return m_instance
    }
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
static process_pool<T>* process_pool<T>:: m_instance=NULL;

#endif
