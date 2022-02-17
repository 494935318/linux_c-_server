#include"log.h"
#include"event_loop.h"
void wrie_log(){

    LOG_TRACE<<"text";
}

void * thread_run(void*){
event_loop loop;
timeval a;
a.tv_sec=0;
a.tv_usec=10;
loop.runevery(a,wrie_log);
loop.run();
}
int main4(){
Logger::start_log();
pthread_t a[10];
for(int i=0;i<10;i++){
    pthread_create(&a[i],NULL,thread_run,NULL);
}
thread_run(NULL);
}
