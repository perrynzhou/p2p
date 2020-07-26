/*************************************************************************
  > File Name: util.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Friday, July 03, 2020 PM02:24:38 HKT
 ************************************************************************/
#include <stddef.h>
int init_tcp_socket(const char *addr, int port,int backlog);
int init_udp_socket(const char *addr,int port);
int init_tcp_client(const char *addr,int port);
int set_tcp_nonblock(int fd);
void fetch_client_address(int client_fd, char *address,size_t  address_size);
int init_uuid(char *buf,size_t buf_size);
