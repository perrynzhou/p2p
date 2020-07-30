/*************************************************************************
  > File Name: util.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Friday, July 03, 2020 PM02:24:38 HKT
 ************************************************************************/
#include <stddef.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
int init_tcp_socket(const char *addr, int port, int backlog);
int init_udp_socket(const char *addr, int port, struct sockaddr_in *addrsrv);
int init_tcp_client(const char *addr, int port);
int set_tcp_nonblock(int fd);
void fetch_ip_address_from_fd(int client_fd, char *address, size_t address_size);
void fetch_ip_address_from_localhost(char *buf,size_t buf_size);
