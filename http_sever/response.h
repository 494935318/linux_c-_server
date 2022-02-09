#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include"utils.h"
#include"base.h"

class response:base_body{
    public:
    void set_head(string a,string b){
        
    };
    string  get_head(string a );
    void set_cookie(string a, string b);
    string  get_cookie(string a);
    void set_data(string a);
    void set_file(int file_fd);
    string get_content();
    response* get_this(){
    return this;
    }
    header& get_header(){
        return head;
    }
    private:
    string data;
    bool has_file;
    int file_fd;
    header head;
}
#endif // __RESPONSE_H__