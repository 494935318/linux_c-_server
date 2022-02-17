#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include "utils.h"
#include "locker.h"

template <class T>
class thread_pool
{

public:
    thread_pool(int thread_num = 8, int max_requests = 1000);
    ~thread_pool()
    {
        delete[] m_threads;
        m_stop = true;
    };
    bool append(T *request);

private:
    static void *worker(void *arg);
    void run();

    int m_thread_number;
    int m_max_requests;
    pthread_t *m_threads;
    list<T*> m_workqueue;
    locker m_queuelocker;
    sem m_queuestat;
    bool m_stop;
};
template <class T>
thread_pool<T>::thread_pool(int thread_num, int max_requests) : m_thread_number(thread_num), m_max_requests(max_requests), m_stop(false), m_threads(NULL)
{
    assert(thread_num > 0 && max_requests > 0);
    m_threads = new pthread_t[m_thread_number];
    assert(m_threads);
    for (int i = 0; i < thread_num; ++i)
    {
        // syslog(LOG_DEBUG, "create the %dth thread\n", i);
        if (pthread_create(&m_threads[i], NULL, worker, this) != 0)
        {
            delete[] m_threads;
            throw std::exception();
        }
        if (pthread_detach(m_threads[i]))
        {
            delete[] m_threads;
        }
    }
}
template <class T>
bool thread_pool<T>::append(T *request)
{   {
    lock_guard tmp(m_queuelocker);
    if (m_workqueue.size() > m_max_requests)
    {
        return false;
    }
    m_workqueue.push_back(request);
    }
    m_queuestat.post();
    return true;
}
template <class T>
void *thread_pool<T>::worker(void *arg)
{
    thread_pool *pool = (thread_pool *)arg;
    pool->run();
    return pool;
}
template <class T>
void thread_pool<T>::run()
{
    while(! m_stop)
    {   
        m_queuestat.wait();
        lock_guard tmp(m_queuelocker);
        if(m_workqueue.empty())
        {
            
            continue;
        }
        T* request=m_workqueue.front();
        m_workqueue.pop_front();
        tmp.unlock();
        if(! request){
            continue;
        }
        request->process();

    }
}
#endif // __THREAD_POOL_H__

