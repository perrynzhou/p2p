/*************************************************************************
  > File Name: tcp_server.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Friday, July 03, 2020 PM02:24:38 HKT
 ************************************************************************/

#include "tcp_server.h"
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
int tcp_server_init(struct tcp_server *ts, const char *addr, int port, int max_connections)
{
  if (ts != NULL)
  {
    ts->max_connections = max_connections;
    int sfd = init_tcp_socket(addr, port, 0);
    if (sfd == -1 || (ts->efd = epoll_create(max_connections)) == -1)
    {
      return -1;
    }
    int reuse = 1;
    setsockopt(ts->sfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(int));
    ts->sfd = ts->event.data.fd = sfd;
    ts->event.events = EPOLLIN | EPOLLET;
    epoll_ctl(ts->efd, EPOLL_CTL_ADD, sfd, &ts->event);
    ts->connections_events = calloc(max_connections, sizeof(struct epoll_event));
    if (ts->connections_events == NULL)
    {
      close(ts->efd);
      close(ts->sfd);
      return -1;
    }
    return 0;
  }
}
int tcp_server_start(struct tcp_server *ts)
{
  struct hash_list *list = hash_list_alloc(4096);
  while (1)
  {
    int n = epoll_wait(ts->efd, ts->connections_events, ts->max_connections, -1);
    for (int i = 0; i < n; i++)
    {
      if (ts->connections_events[i].events & EPOLLERR)
      {
        fprintf(stdout, "epoll error\n");
        close(ts->connections_events[i].data.fd);
        continue;
      }
      else if (ts->sfd == ts->connections_events[i].data.fd)
      {
        struct sockaddr client_addr;
        socklen_t len = sizeof(struct sockaddr);
        int client_fd = accept(ts->sfd, &client_addr, &len);
        if (client_fd == -1)
        {
          break;
        }
        set_tcp_nonblock(client_fd);
        ts->event.data.fd = client_fd;
        ts->event.events = EPOLLIN | EPOLLET;
        epoll_ctl(ts->efd, EPOLL_CTL_ADD, client_fd, &ts->event);
        fprintf(stdout, "accept \n");
      }
      else
      {

        connection_meta tmp;
        ssize_t count = recv(ts->connections_events[i].data.fd, &tmp, sizeof(tmp), 0);
        if (count > 0)
        {
          char addr[32] = {'\0'};
          fetch_client_address(ts->connections_events[i].data.fd, (char *)&addr, 32);
          if (tmp.kind == connection_in)
          {
            connection_meta *cm = connection_meta_alloc(tmp.kind, (char *)&tmp.addr);
            hash_list_insert(list, (char *)&tmp.addr, cm);
            fprintf(stdout, "%s connected\n", (char *)&addr);
          }
          else
          {
            hash_list_remove(list, (char *)&tmp.addr);
            epoll_ctl(ts->efd, EPOLL_CTL_DEL, ts->connections_events[i].data.fd, &ts->event);
            fprintf(stdout, "%s leave\n", (char *)&addr);
          }
        }
      }
    }
  }
}
int main(int argc, char *argv[])
{
  struct tcp_server ts;
  memset(&ts, 0, sizeof(ts));
  tcp_server_init(&ts, argv[1], atoi(argv[2]), 1024);
  tcp_server_start(&ts);
  return 0;
}
