/*************************************************************************
  > File Name: tcp_server.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Friday, July 03, 2020 PM02:24:38 HKT
 ************************************************************************/

#include "message.h"
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
#include "util.h"
#include "hash_list.h"
#include "message.h"
int main(int argc, char *argv[])
{
  int fd = init_tcp_socket(argv[1], atoi(argv[2]),0);
  assert(fd != -1);
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
  struct hash_list *list = hash_list_alloc(4096);
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
        set_tcp_nonblock(client_fd);
        event.data.fd = client_fd;
        event.events = EPOLLIN | EPOLLET;
        epoll_ctl(efd, EPOLL_CTL_ADD, client_fd, &event);
        fprintf(stdout,"accept \n");
      }
      else
      {
        char addr[32] = {'\0'};
        fetch_client_address(events[i].data.fd, (char *)&addr, 32);
        struct connection_message tmp;
        ssize_t count = recv(events[i].data.fd, &tmp, 4096, 0);
        if (count > 0)
        {
          char *uuid = (char *)&tmp.uuid;
          if (tmp.kind == connection_in)
          {
            struct connection_message *cm = connection_message_alloc(tmp.kind,uuid);
            hash_list_insert(list, uuid,cm);
            fprintf(stdout, "%s connected\n", (char *)&addr);
          }
          else
          {
            hash_list_remove(list, uuid);
            epoll_ctl(efd, EPOLL_CTL_DEL, events[i].data.fd, &event);
            fprintf(stdout, "%s leave\n", (char *)&addr);
          }
        }
      }
    }
  }
}
