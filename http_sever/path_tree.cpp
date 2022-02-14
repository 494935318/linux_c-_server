#include"path_tree.h"
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
    return out;
}
bool path_tree:: has_path(const string &a){
        return get_cb(a)==0;
    };
    void path_tree::add_path(const string &a,http_cb b){
          auto path=parse_path(a);
        auto tmp=header;
        int i=0;
        if(path.size()==1 &&path[0]==""){
            header->cb=b;
            header->is_path=true;
        }
        else
        while(i<path.size()&&tmp){
            auto now=path[i++];
            auto j=tmp->next.find(now);
            if(j!=tmp->next.end())
            tmp=j->second;
            else{
                node * now_node=new node();
                tmp->next[now]=now_node;
                tmp=now_node;
            }
        }
        tmp->cb=b;
        tmp->is_path=true;
    };
    http_cb path_tree::get_cb(string a){
          auto path=parse_path(a);
        auto tmp=header;
        int i=0;
        http_cb out=0;
        if(path.size()==1 &&path[0]==""){
            if(header->is_path)
            return header->cb;
            else return 0;
        }else
        while(i<path.size()&&tmp){
            auto now=path[i++];
            auto j=tmp->next.find(now);
            if(j==tmp->next.end()){
                return out;
            }
            tmp=j->second;
            if(tmp->is_path) out=tmp->cb;
        }
       return out;
    };