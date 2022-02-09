#ifndef __BASE_H__
#define __BASE_H__

#include"utils.h"
class header
{
    public:
        string get_data(string a);
        void set_data(string a,string b);
        void set_cookie(string a, string b );
        void get_cookie(string a ,string b);
    private:
        unordered_map<string ,string > head_data;
        unordered_map<string ,string > cookies;
};

class  base_body{
public:
virtual ~base_body()=0;
virtual base_body* get_this(){
    return this;
    }
virtual string get_content()=0;
virtual header& get_header()=0;
void set_conext(string a,any b){
    context[a]=b;
    }
any get_context(string a){
    return context[a];
}
void del_context(string a){
    context.erase(a);
}
private :
unordered_map<string ,any> context;
};


#endif // __BASE_H__