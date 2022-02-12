#include"Time_wheel.h"
namespace Sever_time_wheel{

Entry::Entry(shared_ptr<TCP_Connect> a){
con=a;
};
Entry::~Entry(){
    auto tmp=con.lock();
        if(tmp){
            tmp->shurtdown();
        }
};

}