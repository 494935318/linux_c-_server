#include"all_include.h"

// sockaddr* get_tcp_address(char * ip,int port);
sockaddr_in* get_addr_ipv4(const char * ip,int port);
sockaddr_in6* get_addr_ipv6(const char * ip,int port);
void print_getsockname(int fd);
void print_getpeername(int fd);