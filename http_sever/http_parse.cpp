#include"http_parse.h"
LINE_STATUS parse_line(char *buff, int &checked_index, int read_index)
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
