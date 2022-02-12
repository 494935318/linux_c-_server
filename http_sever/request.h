#ifndef __REQUEST_H__
#define __REQUEST_H__

#include "utils.h"
#include "http_parse.h"
#include "base.h"
#include "TCP_Connect.h"
#include "config.h"
class request : public base_body
{
public:
    request(){

    };
    ~request(){
        
    };
    void set_getdata(string a){
        get_data=a;
    };
    string get_getdata(){
        return get_data;
    }
    void set_method(string a)
    {
        method = a;
    };
    void set_url(string a)
    {
        url = a;
    };
    void set_http_ver(string a)
    {
        http_version = a;
    };
    // void set_host(string a);
    void set_head_data(string a, string b)
    {
        head_data.set_data(a, b);
    };
    void set_host(string a)
    {
        host = a;
    }
    void set_contentLength(int a)
    {
        content_length = a;
    }
    int get_contentLength()
    {
        return content_length;
    }
    bool set_content(string a, int n)
    {
        post_data += a;
        content_readed += n;
        return content_length == content_readed;
    }

    string get_method()
    {
        return method;
    };
    string get_url()
    {
        return url;
    };
    string get_http_ver()
    {
        return http_version;
    };
    string get_host()
    {
        return host;
    };
    string get_head_data(string a)
    {
        return head_data.get_data(a);
    };
    string get_content()
    {
        return content;
    };
    string add_content(string a)
    {
        content += a;
    };
    request *get_this()
    {
        return this;
    }
    header &get_header()
    {
        return head_data;
    }
    HTTP_CODE parse_requestion(weak_TCP in)
    {
        auto tmp = in.lock();
        if (tmp)
        {
            LINE_STATUS linestatus = LINE_OK;
            HTTP_CODE retcode = NO_REQUEST;
            if (checkstat != CHECK_STATE_CONTENT)
            {
                while ((linestatus = parse_line(tmp->read_buf.begin(), checked_index, tmp->read_buf.readable_size())) == LINE_OK)
                {
                    string line = string(tmp->read_buf.begin(), tmp->read_buf.begin() + checked_index);
                    tmp->read_buf.retrieve(checked_index);
                    checked_index = 0;
                    switch (checkstat)
                    {
                    case CHECK_STATE_REQUESTLINE:
                    {
                        retcode = ::parse_requestion<request>(line.data(), checkstat, *this);
                        if (retcode == BAD_REQUEST)
                        {
                            return BAD_REQUEST;
                        }
                        break;
                    }

                    case CHECK_STATE_HEADER:
                    {
                        retcode = parse_headers<request>(line.data(), *this,checkstat);
                        if (retcode == BAD_REQUEST)
                        {
                            return retcode;
                        }
                        else if (retcode == GET_REQUEST)
                        {
                            return retcode;
                        }
                        break;
                    }
                    default:
                    {
                        return INTERNAL_ERROR;
                    }
                    }
                }
                if (linestatus == LINE_OPEN)
                {
                    return NO_REQUEST;
                }
                else
                {

                    return BAD_REQUEST;
                }
            }
        
            else{
                if(tmp->read_buf.readable_size()>=content_length)
                {
                content=string(tmp->read_buf.begin(),content_length);
                tmp->read_buf.retrieve(content_length);
                return GET_REQUEST;
                }
                else 
                return NO_REQUEST;
            }
        };
    }
 void reset()
    {
        checkstat = CHECK_STATE_REQUESTLINE;
        checked_index = 0;
        content_readed = 0;
        content_length = 0;
        content.clear();
    }
private:
   
    int checked_index = 0;
    int content_length = 0;
    int content_readed = 0;
    CHECK_STATE checkstat = CHECK_STATE_REQUESTLINE;
    string method;
    string get_data;
    string url;
    string http_version;
    string host;
    header head_data;
    string post_data;
    string content;
};
#endif // __REQUEST_H__