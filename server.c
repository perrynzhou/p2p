/*************************************************************************
  > File Name: server.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Friday, July 03, 2020 PM02:24:38 HKT
 ************************************************************************/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <assert.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "utils.h"
#include "hash_list.h"
#include "hashfn.h"
typedef struct client_fd_meta_t
{
  char *addr;
  int fd;
  int8_t  status;
} client_fd_meta;
static void travel_cb(void *p_client_fd_meta, void *data);
static int init_tcp_socket(const char *addr, int port)
{
  int fd = socket(PF_INET, SOCK_STREAM, 0);
  struct sockaddr_in serveraddr;
  serveraddr.sin_family = AF_INET;
  inet_pton(AF_INET, addr, &serveraddr.sin_addr);
  serveraddr.sin_port = htons(port);
  bind(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
  listen(fd, 1024);
  return fd;
}
static int set_tcp_socket(int fd)
{

  int flags, s;
  flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1)
  {
    perror("fcntl");
    return -1;
  }
  flags |= O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flags) == -1)
  {
    perror("fcntl");
    return -1;
  }

  return 0;
}

void accept_fd_callback(hash_list *list, int cfd, char *address)
{
  //msg format: 127.0.0.1:8080
  uint32_t hash = hash_gfs(address, strlen(address));
  int index = hash % 4096;
  client_fd_meta *meta = malloc(sizeof(*meta));
  meta->fd = cfd;
  meta->addr = address;
  hash_list_insert(list, address, meta);
}
void close_fd_callback(hash_list *list, char *address)
{
  //msg format: 127.0.0.1:8080
  uint32_t hash = hash_gfs(address, strlen(address));
  int index = hash % 4096;
  hash_list_remove(list, address);
}
void parse_fd_address(int client_fd, char *address, size_t address_size)
{
  struct sockaddr_in addr;
  socklen_t addr_size = sizeof(struct sockaddr_in);
  int res = getpeername(client_fd, (struct sockaddr *)&addr, &addr_size);
  strncpy(address, inet_ntoa(addr.sin_addr), address_size);
  size_t alen = strlen((char *)&address);
  snprintf((char *)&address + alen, address_size - alen, ":%d", htons(addr.sin_port));
}
size_t pack_message(char *buffer, char *address, size_t address_len, int message_type)
{
  message *m = (message *)buffer;
  m->type = message_type;
  m->len = address_len;
  strncpy((char *)buffer + sizeof(message), (char *)address, address_len);
  return strlen((char *)&buffer);
}
static void travel_cb(void *p_client_fd_meta, void *data)
{
  client_fd_meta *cm = (client_fd_meta *)p_client_fd_meta;
  char *addr = (char *)data;
  char buffer[2048] = {'\0'};
  size_t len = pack_message((char *)&buffer, addr, strlen(addr), REGISTER);
  write(cm->fd, (char *)&buffer, len);
}
void do_request(hash_list *list, int efd, int cfd, char *buf)
{
}
int main(int argc, char *argv[])
{
  int fd = init_tcp_socket(argv[1], atoi(argv[2]));
  assert(fd != -1);
  set_tcp_socket(fd);
  int reuse = 0;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(int));
  int efd = epoll_create(1024);
  int max_event = 1024;
  struct epoll_event event;
  event.data.fd = fd;
  event.events = EPOLLIN | EPOLLET;
  epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event);
  struct epoll_event *events = calloc(max_event, sizeof(struct epoll_event));

  //
  hash_list *list = hash_list_alloc(4096);
  while (1)
  {
    int n = epoll_wait(efd, events, max_event, -1);
    for (int i = 0; i < n; i++)
    {
      if (events[i].events & EPOLLERR)
      {
        fprintf(stdout, "epoll error\n");
        close(events[i].data.fd);
        continue;
      }
      else if (fd == events[i].data.fd)
      {
        struct sockaddr client_addr;
        socklen_t len = sizeof(struct sockaddr);
        int client_fd = accept(fd, &client_addr, &len);
        if (client_fd == -1)
        {
          break;
        }
        set_tcp_socket(client_fd);

        char address[128] = {'\0'};
        parse_fd_address(client_fd, (char *)&address, 128);
        hash_list_travel(list, (char *)&address, (hash_list_travel_cb)&travel_cb);
        accept_fd_callback(list, client_fd, (char *)address);
        event.data.fd = client_fd;
        event.events = EPOLLIN | EPOLLET;
        epoll_ctl(efd, EPOLL_CTL_ADD, client_fd, &event);
      }
      else
      {
        while (1)
        {
          char buf[4096] = {'\0'};
          int count = recv(events[i].data.fd, (char *)&buf, 4096, 0);
          if (count < 0)
          {
             char address[128] = {'\0'};
             int client_fd = events[i].data.fd;
             parse_fd_address(client_fd, (char *)&address, 128);
           
            epoll_ctl(efd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
            close(events[i].data.fd);
            continue;
          }
          if (count > 0)
          {
            do_request(list, efd, events[i].data.fd, buf);
          }
        }
      }
    }
  }
}
