#ifndef __PATH_TREE_H__
#define __PATH_TREE_H__

#include"request.h"
#include"response.h"

typedef function<void (request&,response&)> http_cb;
vector<string>  parse_path(const string &in){
    vector<string> out;
    if(in=="/") {
        return {""};
    }
    string a="";
    int n=in.size();
    for(int i=1;i<n;i++){
        if(in[i]=='/'){
            out.push_back(a);
        }
        else{
            a+=in[i];
        }
    }
    if(in.back()!='/'){
        out.push_back(a);
    }
}
class path_tree{
    public:
    bool has_path(const string &a){
        return get_cb(a)==0;
    };
    void add_path(const string &a,http_cb b){
          auto path=parse_path(a);
        auto tmp=header;
        int i=0;
        while(i<path.size()&&tmp){
            auto now=path[i++];
            tmp=tmp->next[now];
        }
        tmp->cb=b;
        tmp->is_path=true;
    };
    http_cb get_cb(string a){
          auto path=parse_path(a);
        auto tmp=header;
        int i=0;
        http_cb out=0;
        while(i<path.size()&&tmp){
            auto now=path[i++];
            if(tmp->next.find(now)==tmp->next.end()){
                return out;
            }
            tmp=tmp->next[now];
            if(tmp->is_path) out=tmp->cb;
        }
       return out;
    };
    private:
    struct node
    {
      unordered_map<string, node*> next;
      bool is_path;
      http_cb cb=0;
    };
    node* header;
    
};

#endif // __PATH_TREE_H__