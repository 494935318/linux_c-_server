#include "utils.h"
#include"http_server.h"

void cb(request &a,response& b){
    b.set_status(200,"OK");
    b.set_data("ok");
}

int main(){
    http_server serve;
    serve.Init(3026,5);
    serve.set_location("/",cb);
    serve.start();
}