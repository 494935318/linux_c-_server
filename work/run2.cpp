#include "utils.h"
#include"http_server.h"
#include<signal.h>
#include"log.h"
void cb(request &a,response& b){
    b.set_status(200,"OK");
    string data="ok";
    b.set_data(data);
}

int main(){
    // Logger::start_log();
    Logger::setLogLevel(Logger::LogLevel::TRACE);
    LOG_DEBUG<<"start:"<<"\n";
    sigset_t signal_mask;
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGPIPE);
    if(pthread_sigmask(SIG_BLOCK, &signal_mask, NULL) == -1)
    perror("SIG_PIPE");

    // IgnoreSigPipe();
    http_server serve;
    serve.Init(3026,5);
    serve.set_location("/",cb);
    serve.start();
   
}