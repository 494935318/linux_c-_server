#include"utils.h"
class lock_guard;
class cond;
class sem{
    public:
    sem(){
        if(sem_init(&m_sem,0,0)!=0)
        {
            throw  std::exception();
        }
    }
    ~sem(){
        sem_destroy(&m_sem);
    }
    bool wait()
    {
        return sem_wait(&m_sem)==0;
    }
    bool post()
    {
        return sem_post(&m_sem)==0;
    }
    private:
    sem_t m_sem;
};
class locker:noncopyable
{
public:
    locker()
    {
        pthread_mutex_init(&m_mutex,NULL);
    }
    ~locker(){
        assert(hold==0);
        pthread_mutex_destroy(&m_mutex);

    }
    friend class cond;
 friend class lock_guard;
private:
    pthread_mutex_t m_mutex;
   mutable pid_t hold=0;
    bool lock(){
        
        int i=pthread_mutex_lock(&m_mutex);
        assert(i==0);
        hold=pthread_self();
        return i==0;
    }
    bool unlock(){
        hold= 0;
        return pthread_mutex_unlock(&m_mutex)==0;
    }
    
};

class cond:noncopyable
{
    public:
    cond(){
        pthread_cond_init(&m_cond,NULL);
    }
    template<class T>
    bool wait(locker &a,const T &func){
        int ret=0;
        while(! func()){
            ret=pthread_cond_wait(&m_cond,&a.m_mutex);
            a.hold=pthread_self();
        }
        return ret==0;
    }
    bool signal(){
        return pthread_cond_signal(&m_cond)==0;
    }
    bool broadcast(){
        return pthread_cond_broadcast(&m_cond)==0;
    }
    private:
    pthread_cond_t m_cond;
};
class lock_guard:noncopyable
{
    public:
        lock_guard(locker &a):b(a){
          assert(  b.lock());
            locked=true;
        }
        ~lock_guard(){
            if(locked)
           { b.unlock();
           locked=false;
           }
        }
        void unlock(){
            if(locked)
            assert(b.unlock());
        }

    private:
        lock_guard(const lock_guard&);
        lock_guard& operator =(const lock_guard&);
        locker&b;
        bool locked=true;
};
template<class T> 
class block_queue:noncopyable
{
    private:
        sem s;
        queue<T> q;
        locker l;
    public:
        void push(T a){
            l.lock();
            q.push(a);
            s.post();
            l.unlock();
        }
        T get(){
            s.wait();
            l.lock();
            T out=q.top();
            q.pop();
            l.unlock();
            return out;
        }
};

class count_down_latch:noncopyable
{
    public:
        explicit count_down_latch(int i):count_num(i){

        };
        void count_down(){
            lock_guard c(a);
            count_num--;
            if(count_num==0){
                b.broadcast();
            }
        };
      
        void wait(){             
            auto d=bind(&count_down_latch::func,this);
            lock_guard e(a);
            b.wait(a,d);
        }
    private:
        bool func(){
            if(count_num==0)return true;
            return false;
        }
        int count_num;
        locker a;
        cond b;
};
