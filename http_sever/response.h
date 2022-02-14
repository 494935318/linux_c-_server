#ifndef __RESPONSE_H__
#define __RESPONSE_H__ 0

#include"utils.h"
#include"base.h"
static const char *ok_200_title = "OK";
static const char *error_400_title = "Bad Request";
static const char *error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
static const char *error_403_title = "Forbidden";
static const char *error_403_form = "You do not have permission to get file form this server.\n";
static const char *error_404_title = "Not Found";
static const char *error_404_form = "The requested file was not found on this server.\n";
static const char *error_500_title = "Internal Error";
static const char *error_500_form = "There was an unusual problem serving the request file.\n";
class response:base_body{
    public:
    void set_head_data(string a,string b){
        head.set_data(a,b);
    };
    void set_status(int a,const char * word)
    {   char tmp[1000];
        sprintf(tmp,"%s %d %s\r\n", "HTTP/1.1", a, word);
        status=tmp;
    }
    string  get_head(string a ){
        return head.get_data(a);
    };
    void set_cookie(string a, string b){
        head.set_cookie(a,b);
    };
    string  get_cookie(string a){
        return head.get_cookie(a);
    };
    void set_data(const string &a){
        is_file_data=false;
        set_content_length(a.size());
        data=a;
    };
    void set_file(int file_fd,int size){
        is_file_data=true;
        set_content_length(size);
        this->file_fd=file_fd;
    };
    string get_content(){
        string out="";
        out+=status+length()+head.get_content()+"\r\n"+data;
        return out;
    };
    response* get_this(){
    return this;
    }
    header& get_header(){
        return head;
    }
    void set_content_length(long long a){
        content_length=a;
    };
    long long get_content_length(){
        return content_length;
    }
    
    private:
    string length(){
        char tmp[1000]={'\0'};
        sprintf(tmp,"Content-Length:%lld\r\n", content_length);
        return tmp;
    }
    bool is_file_data=false;
    int file_fd;
    string  status;
    string data;
    bool has_file;
    // int file_fd;
    long long content_length=0;
    header head;
}

;
class Response_404:public response{
    public:
     Response_404(string a){
        set_data(a);
        set_status(404,error_404_title);
    }
};
class Response_403:public response{
    public:
     Response_403(string a){
        set_data(a);
        set_status(403,error_403_title);
    }
};
class Response_500:public response{
    public:
     Response_500(string a){
        set_data(a);
        set_status(500,error_500_title);
    }
};



#endif // __RESPONSE_H__