#include "http_server.h"
void http_server::Init(int port,int thread_num){
    loop.reset(new event_loop());
    server.reset(new TCP_Server(loop.get(),"0.0.0.0",port));
    if(thread_num>0)
    server->set_threadnum(thread_num);
    server->set_on_connect( bind(&http_server::on_connect,this,placeholders::_1));
    server->set_on_message(0);
};
void http_server::start(){
    server->work();
    loop->run();
};
void http_server::set_location(string locate, http_cb cb){
    tree.add_path(locate,cb);
};
void  http_server::on_connect(weak_TCP in){
    auto tmp=in.lock();
    if(tmp){

shared_ptr<request> req(new request());
tmp->set_on_message(bind(&http_server::on_data,this,placeholders::_1,req));

    }


}
 void http_server::on_data(weak_TCP in,shared_ptr<request> req){
    auto tmp=in.lock();
    if(tmp){
    auto stat=req->parse_requestion(in);
    if(stat==GET_REQUEST){
        auto cb=tree.get_cb(req->get_url());
        if(cb!=0){
            response out;
            cb(*req,out);
            tmp->send(out.get_content());
            req->reset();
        }
        else{
            auto a=Response_404("bad_request").get_content();
             tmp->send(a);
             req->reset();

        }
        }
    else if(stat==BAD_REQUEST){
         auto a=Response_404("bad_request").get_content();
        tmp->send(a);
        req->reset();

    }
    }
};