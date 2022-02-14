#include "utils.h"
#include"http_server.h"
#include<signal.h>
void cb(request &a,response& b){
    b.set_status(200,"OK");
    string data="ok";
    b.set_data(data);
}

int main(){
    // ::signal(SIGPIPE,SIG_IGN);
    sigset_t signal_mask;
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGPIPE);
    if(pthread_sigmask(SIG_BLOCK, &signal_mask, NULL) == -1)
    perror("SIG_PIPE");

    // IgnoreSigPipe();
    http_server serve;
    serve.Init(3026,8);
    serve.set_location("/",cb);
    serve.start();
    // vector<int> a;
    // nth_element();
    
}