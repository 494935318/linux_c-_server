#include"utils.h"
#include"event_loop.h"
class TCP_Connect;
vector<shared_ptr<TCP_Connect>> mem_TCP;
class TCP_Connect:noncopyable,public enable_shared_from_this<TCP_Connect>
{
    public:
    typedef shared_ptr<TCP_Connect> TCP_ConnectPtr;
    TCP_Connect()
    {

    };
    friend class TCP_ConnectPtr;
    
    private:
    int _is_long=false;
    ~TCP_Connect (){
        delete []writebuff;
        delete []readbuff;
        close(fd);
    }
    int fd;
    char* writebuff;
    char * readbuff;
    int epoll_fd;
    shared_ptr<event_loop> loop;
};