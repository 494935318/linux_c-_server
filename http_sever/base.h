#ifndef __BASE_H__
#define __BASE_H__

#include"utils.h"
#include<string.h>
#include<stdio.h>
class header
{
    public:
        string get_data(string a){
            auto j=head_data.find(a);
            if(j!=head_data.end())
            return j->second;
            else{
                return "";
            }
        };
        void set_data(string a,string b){
            head_data[a]=b;
        };
        void set_cookie(string a, string b ){
            cookies[a]=b;
        };
        string  get_cookie(string a ){
            auto j=cookies.find(a);
            if(j!=cookies.end())
            return j->second;
            else{
                return "";
            }
        };
        string get_content(){
            string out="";
            // char tmp[10000]={'\0'};
            for(auto i=head_data.begin();i!=head_data.end();i++){
                // char tmp[10000]={'\0'};
                // sprintf(tmp,"%s:%s\r\n",(*i).first,(*i).second);
                out+=(*i).first+':'+(*i).second+"\r\n";
                // fill(tmp,&tmp[10000],'\0');
            }
            if(cookies.size()!=0){
                string cookies_out="cookie: ";
                for(auto i=cookies.begin();i!=cookies.end();i++){
                    cookies_out+= (*i).first+'='+(*i).second+' ';
                };
                cookies_out+="\r\n";
                out+=cookies_out;
            }
            return out;
        }
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