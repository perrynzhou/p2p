
/*************************************************************************
  > File Name: util.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Friday, July 03, 2020 PM02:24:38 HKT
 ************************************************************************/
#include "util.h"
#include "hash_list.h"
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <uuid/uuid.h>
#define NET_BACKLOG_LEN (1024)
static int init_socket(int domain, int type, int protocol, int backlog,
                       const char *addr, int port)
{
  int sock = socket(domain, type, protocol);
  if (sock != -1)
  {
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = domain;
    serveraddr.sin_port = htons(port);
    bind(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (type == SOCK_STREAM)
    {
      int opt = 1;
      setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
      set_tcp_nonblock(sock);
      listen(sock, backlog);
    }
  }
  return sock;
}
int init_tcp_client(const char *addr, int port)
{
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock != -1)
  {
    struct sockaddr_in srvaddr;
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, addr, &srvaddr.sin_addr) <= 0)
    {
      close(sock);
      printf("inet_pton error for %s\n", addr);
      return -1;
    }
    if (connect(sock, (struct sockaddr *)&srvaddr, sizeof(srvaddr)) != 0)
    {
      fprintf(stdout, "connect %s:%d :%s\n", addr, port, strerror(errno));
      close(sock);
      sock = -1;
    }
  }
  return sock;
}
int init_tcp_socket(const char *addr, int port, int backlog)
{
  int real_backlog = (backlog < NET_BACKLOG_LEN) ? NET_BACKLOG_LEN : backlog;
  return init_socket(AF_INET, SOCK_STREAM, 0, real_backlog, addr, port);
}
int init_udp_socket(const char *addr, int port)
{
  return init_socket(AF_INET, SOCK_DGRAM, 0, 0, addr, port);
}
int set_tcp_nonblock(int fd)
{
  int fl = fcntl(fd, F_GETFL);
  return fcntl(fd, F_SETFL, fl | O_NONBLOCK);
}
void fetch_client_address(int client_fd, char *address, size_t address_size)
{
  struct sockaddr_in addr;
  socklen_t addr_size = sizeof(struct sockaddr_in);
  int res = getpeername(client_fd, (struct sockaddr *)&addr, &addr_size);
  strncpy(address, inet_ntoa(addr.sin_addr), address_size);
  size_t alen = strlen((char *)&address);
  snprintf((char *)&address + alen, address_size - alen, ":%d", htons(addr.sin_port));
}
int init_uuid(char *buf, size_t buf_size)
{
  uuid_t uuid;
  uuid_generate_time(uuid);
  snprintf(buf, buf_size, "%s", (char *)uuid);
}