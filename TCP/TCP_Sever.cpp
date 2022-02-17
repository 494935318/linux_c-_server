#include "TCP_Server.h"
#include"TCP_Connect.h"
#include"Acceptor.h"
#include"event_loop.h"
#include"Thread_event_loop.h"
#include"log.h"
TCP_Server::TCP_Server(event_loop * loop, const string & ip, int port, int size):loop(loop)
{
    acceptor_.reset(new Acceptor(ip,port,loop,size));
};
void TCP_Server::set_on_connect(const server_callback_fun& cb){
on_connect=cb;
};
void TCP_Server::set_on_message(const server_callback_fun& cb){
on_message=cb;
};
void TCP_Server::on_new_connect(const int &fd, const sockaddr_in &addr){
     shared_ptr<TCP_Connect> con;
    if(!multi_thread)
    con=make_shared<TCP_Connect>(fd,loop);
    else
    {auto loop_idx=loop_pool->getnext();
    con=make_shared<TCP_Connect>(fd,loop_idx);}
    con->set_on_message(on_message);
    // sever 记录链接的TCP
    connect_map[fd]=con;
    con->set_on_close(bind(&TCP_Server::on_connect_close,this,placeholders::_1));

    // 预备工作
    if(on_connect) on_connect(con);
    con->work();

};
 void  TCP_Server::set_threadnum(int size){
     loop_pool.reset(new Thread_event_loop(size));
    multi_thread=true;
 }
void TCP_Server::on_connect_close(const shared_ptr<TCP_Connect> &tmp){
    if(loop->is_in_loopthread()){
        remove_connect(tmp->get_fd());
        
    }
    else{
        loop->runsooner(bind(&TCP_Server::remove_connect,this,tmp->get_fd()));
    }
};
void TCP_Server::remove_connect(int fd){
    
    connect_map.erase(fd);
    LOG_DEBUG<<"remain connect:"<<connect_map.size();
    //
};
void TCP_Server::work(){
    acceptor_->set_connect_cb(bind(&TCP_Server::on_new_connect,this,placeholders::_1,placeholders::_2));
    acceptor_->listen();
};
