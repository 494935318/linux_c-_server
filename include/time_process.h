#include "utils.h"
#include "config.h"

template<class client>
class tw_timer{
    public:
    tw_timer(int rot,int ts):next(nullptr),prev(nullptr),rotation(rot),time_slot(ts)
    {

    }

    tw_timer * next,*prev;
    int rotation,time_slot;

    void( * cb_func)(client*);
    client*user_data;


};

template<class client,int N=60,int SI=1>
class time_wheel{

    public:
    time_wheel():cur_slot(0) {
        for(int i=0;i<N;i++){
            slots[i]=nullptr;
        }
    }
    ~time_wheel(){
     for(int i=0;i<N;i++){
         tw_timer<client> *tmp=slots[i];
         while(tmp){
             slots[i]=tmp->next;
             delete tmp;
             tmp=slots[i];
         }
     }
    }
    tw_timer<client> * addtimer(int time_out){
        if( time_out<0){
            return nullptr;
        }
        int ticks=0;
        if(time_out<SI){
            ticks=1;
        }
        else{
            ticks=time_out/SI;
        }
        int rotation=ticks/N;   
        int ts=(cur_slot+(ticks%N))%N;
        tw_timer<client> * timer= new tw_timer<client>(rotation ,ts);
        if(!slots[ts]){
            slots[ts]=timer;
        }else{
            timer->next=slots[ts];
            slots[ts]->prev=timer;
            slots[ts]=timer;
            
        }

        return timer;

    }
    void del_timer(tw_timer<client> * timer){
        if(!timer){return ;}
        int ts=timer->time_slot;
        if( timer==slots[ts]){
            slots[ts]=slots[ts]->next;
            if(slots[ts])
            slots[ts]->prev=NULL;
            delete timer;
        }
        else{
            timer->prev->next=timer->next;
            if(timer->next){
                timer->next->prev=timer->prev;
            }
            delete timer;
        }
    }
    void tick(){
        tw_timer<client> * tmp =slots[cur_slot];
        while (tmp)
        {
            if(tmp->rotation>0){
                tmp->rotation--;
                tmp=tmp->next;
            }
            else{
                tmp->cb_func(tmp->user_data);
                auto del_tmp=tmp;
                tmp=tmp->next;
                del_timer(del_tmp);
            }
        }
        cur_slot=++cur_slot%N;
    }
    private:
    int cur_slot;
    tw_timer<client> slots[N];


};
template<class client>
class heap_timer{
    public :
        heap_timer(int delay){
            expire=time(NULL)+delay;
        }
        time_t expire;
        void(* cb_func)(client*);
        client*user_data;
        bool operator >(const heap_timer &b){
            return expire>b.expire;
        }
};
template<class client>
class time_heap{
    typedef heap_timer<client>  timer;
public:
    time_heap(){
        heap=priority_queue<timer*,deque<timer*>,decltype(cmp)>(cmp);
    }
    void add_timer(const timer *timer_add ) {
        if(!timer_add) return ;
        heap.push(timer_add);
    }
    void del_timer(timer *timer_del){
        if(timer_del)
        timer_del->cb_fun=NULL;

    }
    timer * top() const{
        if(!heap.empty()){
            return heap.top();
        }
        return nullptr;
    }
    void pop(){
        if(!heap.empty()){
            delete heap.top();
            heap.pop();
        }
    }
    void tick(){
        auto tmp=top();
        time_t cur= time(NULL);
        while(tmp&&tmp->expire<cur){
            if(tmp->cb_func){
                tmp->cb_func(tmp->user_data);
            }
            pop();
            tmp=top();
        }

    }
private:
    static bool cmp(timer*a, timer*b){
        return  *a>*b;
    }
    priority_queue<timer*,deque<timer*>,decltype(cmp)> heap;


};
