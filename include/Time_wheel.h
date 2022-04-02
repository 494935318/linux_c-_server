#ifndef __TIME_WHEEL_H__
#define __TIME_WHEEL_H__

#include "utils.h"
#include"TCP_Connect.h"
#include"event_loop.h"
class TCP_Connect;
class event_loop;
namespace Sever_time_wheel{
class Entry:noncopyable,public enable_shared_from_this<Entry>{
    Entry(shared_ptr<TCP_Connect> );
    ~Entry();
    weak_ptr<TCP_Connect> con;
};
template<class T>
class time_wheel:noncopyable{
    public:
    time_wheel(event_loop* loop,timeval peer):loop(loop){
        t_id=loop->runevery(peer,bind(&time_wheel<T>::time_handle,this));
    };
    void update(shared_ptr<T> a){
        if(loop->is_in_loopthread())
        time_wheel_list.back().push_back(a);
        else{
            loop->runsooner(bind(&time_wheel<T>::update,this,a));
        }
    };
    ~time_wheel(){
        t_id.stop();
    }
    private:
    event_loop* loop;
    void time_handle(){
        time_wheel_list.pop_back();
        time_wheel_list.push_back(vector<shared_ptr<T>>());
    }
    std::list<vector<shared_ptr<T>>> time_wheel_list;
    time_id  t_id;

};

}
#endif // __TIME_WHEEL_H__