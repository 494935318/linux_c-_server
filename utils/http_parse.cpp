#include "utils.h"
#include "config.h"
LINE_STATUS parse_line(char *buff, int &checked_index, int &read_index)
{
    char temp;
    for (; checked_index < read_index; ++checked_index)
    {
        temp = buff[checked_index];
        if (temp == '\r')
        {
            if (checked_index + 1 == read_index)
            {
                return LINE_OPEN;
            }
            else if (buff[checked_index + 1] == '\n')
            {
                buff[checked_index++] = '\0';
                buff[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if (temp == '\n')
        {
            if (checked_index > 1 && buff[checked_index - 1] == '\r')
            {
                buff[checked_index - 1] = '\0';
                buff[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}
//分析请求行
template <class T>
HTTP_CODE parse_requestion(char *temp, CHECK_STATE &checkstate, T &holder)
{
    char *url = strpbrk(temp, " \t");
    if (!url)
    {
        return BAD_REQUEST;
    }
    *url++ = '\0'; //断开
    char *method = temp;
    strcpy(holder.method, method);
    url += strspn(url, " \t");

    char *version = strpbrk(url, " \t");
    if (!version)
    {
        return BAD_REQUEST;
    }
    *version++ = '\0';
    version += strspn(version, " \t");
    strcpy(holder.version, version);
    holder.set_url(url);
    checkstate = CHECK_STATE_HEADER;
    return NO_REQUEST;
}
template <class T>
HTTP_CODE parse_headers(char *temp, T &holder)
{
    if (temp[0] == '\0')
    {
        return GET_REQUEST;
    }
    else if (strncasecmp(temp, "HOST:", 5) == 0)
    {
        temp += 5;
        temp += strspn(temp, " \t");
        strcpy(holder.host, temp);
    }
    else
    {
        cout << "other header 字段" << endl;
    }
    return NO_REQUEST;
}

template <class T>
HTTP_CODE parse_content(char *buffer, int &checked_index, CHECK_STATE &checkstat, int &read_index, int &start_line, T &holder)
{
    LINE_STATUS linestatus = LINE_OK;
    HTTP_CODE retcode = NO_REQUEST;
    while ((linestatus = parse_line(buffer, checked_index, read_index)) == LINE_OK)
    {
        char *tmp = buffer + start_line;
        start_line=checked_index;
        switch (checkstat)
        {
        case CHECK_STATE_REQUESTLINE:
        {
            retcode = parse_requestion(tmp, checkstat, holder);
            if (retcode == BAD_REQUEST)
            {
                return BAD_REQUEST;
            }
            break;
        }
        case CHECK_STATE_HEADER:
        {
            retcode= parse_headers(tmp, holder);
            if( retcode==BAD_REQUEST){
                return retcode;
            }
            else if(retcode==GET_REQUEST){
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