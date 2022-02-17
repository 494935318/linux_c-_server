#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include "utils.h"
#include"path_tree.h"
#include"request.h"
#include"response.h"
#include"TCP_Connect.h"
#include"event_loop.h"
#include"TCP_Server.h"

typedef function<void (request&,response&)> http_cb;
class http_server:noncopyable
{
    public:
        void Init(int port,int thread_num=0);
        void start();
        void set_location(string locate, http_cb cb);
        void on_data(const shared_ptr<TCP_Connect>& in,const shared_ptr<request> &req);
         void on_connect(const shared_ptr<TCP_Connect>&tmp);
    private:
        path_tree tree;
        shared_ptr<event_loop> loop;
       shared_ptr<TCP_Server>  server;


};
#endif // __HTTP_SERVER_H__