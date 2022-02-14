#ifndef __PATH_TREE_H__
#define __PATH_TREE_H__

#include"request.h"
#include"response.h"

typedef function<void (request&,response&)> http_cb;
vector<string>  parse_path(const string &in);
class path_tree{
    public:
    path_tree(){
        header=new node();
    }
    bool has_path(const string &a);
    void add_path(const string &a,http_cb b);
    http_cb get_cb(string a);
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