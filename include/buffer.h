#ifndef __BUFFER_H__
#define __BUFFER_H__

#include"utils.h"
#include<sys/uio.h>
static const int  INIT_SIZE=65535;
static const int PRE_SIZE=8;
namespace Buffer{
    
class Buffer:noncopyable{
    public: Buffer(int size=INIT_SIZE,int pre_size=PRE_SIZE):presize(pre_size),read_index(pre_size),write_index(pre_size),buff_(vector<char>(size,'\0')){

    }
    int writeable_size(){
        return buff_.size()-write_index;
    }  
    //尽量 一次性读文文件
    int read_fd(int fd)
    {   char tmp_mem[65536];
        iovec vec[2];
        int all_read=0;
        while(true){
        int write_able_=writeable_size();
        
        vec[1].iov_base=&tmp_mem[0];
        vec[1].iov_len=sizeof(tmp_mem);
        vec[0].iov_base=&buff_[write_index];
        vec[0].iov_len=write_able_;
        int  num_read=readv(fd,vec,2);
        all_read+=num_read;
        if(num_read>write_able_){
            num_read-=write_able_;
            write_index=buff_.size();
            resize(num_read);
           copy(&tmp_mem[0],&tmp_mem[num_read],&buff_[write_index]);
           write_index+=num_read;
           if(num_read<sizeof(tmp_mem)) break;
        }
        else{
        write_index=write_index+num_read;
        break;
        }
        
        }
        return  all_read;
    }
    // 返回第一个可读信息的指针
    char* peek(){
        return  &buff_[read_index];
    }
    int retrieve(int k){
        assert(k+read_index<=write_index);
        read_index+=k;
        clear();
    }
    void retrieve_all(){
        read_index=presize;
        write_index=presize;
        clear();
    }
    // 收缩buff_大小
    void shrink(){
        buff_.resize(write_index>INIT_SIZE?write_index:INIT_SIZE);
    }
    // 清除
    bool clear(){
        if(read_index==write_index){
            read_index=presize;
            write_index=presize;
        }
        // fill(buff_.begin()+PRE_SIZE,buff_.end(),'\0');
    }
    //末尾增加信息
    // 由send调用
    int append(const string & a){
        append(&a[0],a.size());
        return a.size();
    }
    int append(const char * a,int s){
        if(s>writeable_size()){
            resize(s);
        }
        copy(a,a+s,&buff_[write_index]);
        write_index+=s;
        return s;
    }
  // 在头部添加信息
    int prepend(const string &a){
        prepend(&a[0],a.size());
    }
    int prepend(const char * a,int s){
        assert(s<=read_index);
        read_index-=s;
        copy(a,a+s,&buff_[read_index]);
    }
    // 发送一次消息
    //返回剩下的大小
    int sendfd( int fd){
        // cout<<buff_[read_index]<<endl;
      
        int num= send(fd,&buff_[read_index],readable_size(),MSG_DONTWAIT|MSG_NOSIGNAL);
        read_index+=num;
        clear();
        return num;
    }
    // 
    bool is_empty(){
        return readable_size()==0;
    }
    char * begin(){
        return &buff_[read_index];
    }
    char * end(){
        return &buff_[write_index];
    }
    int readable_size(){
        return write_index-read_index;
    }
    int prependable_size(){
        return read_index;
    }
    int set_size(int n){
        buff_.resize(n+presize);
        if(write_index>buff_.size()){
            write_index=buff_.size();
        }
    }
    
    private:
    // 重置大小
    // 乘以二 , 前挪
    
    int resize(int num){
        if( read_index+writeable_size()>num+presize){
            int tmp=readable_size();
              copy(&buff_[read_index],&buff_[write_index],&buff_[presize]);
              read_index=presize;
              write_index= read_index+tmp;
        }
        else{
             int presize=buff_.size();
           buff_.resize((buff_.size()+num-writeable_size())*2);
        }
        return num;
    }
    
   
    bool has_pre=false;
    vector<char> buff_;
    int read_index;
    int write_index;
    int presize;
};
template<int N,int Pre>
class Fixed_Buffer:public Buffer{
    public:
         Fixed_Buffer(int presize=Pre):Buffer(N,presize){
         }
         int avail(){
             return writeable_size();
         }
         void write_fd(int fd){
             write(fd,begin(),readable_size());
             retrieve_all();
         }
         void reset(){
             retrieve_all();
         }
};

}
#endif // __BUFFER_H__