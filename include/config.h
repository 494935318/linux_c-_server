#ifndef __CONFIG_H__
#define __CONFIG_H__

#define BUFFER_SIZE  4096
#define EVENT_IN 1
#define EVENT_OUT 2
enum CHECK_STATE{ CHECK_STATE_REQUESTLINE=0,CHECK_STATE_HEADER};
enum LINE_STATUS {LINE_OK,LINE_BAD,LINE_OPEN};
enum HTTP_CODE{NO_REQUEST,GET_REQUEST,BAD_REQUEST,FORBIDDEN_REQUEST,INTERNAL_ERROR,CLOSED_CONNECTION};


#endif // __CONFIG_H__