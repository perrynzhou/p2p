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
int tcp_server_send_connection_meta(void *ctx, void *data)
{
  connection_meta *meta = (connection_meta *)data;
  int *fd = (int *)ctx;
  if (send(*fd, meta, sizeof(*meta), 0) > 0)
  {
    fprintf(stdout, "push %s to client success\n", meta->addr);
  }
  return 0;
}
static int tcp_server_push_to_client(tcp_server *ts, int fd, connection_meta *cm)
{
  if (ts != NULL && cm != NULL)
  {
    int client_fd = fd;
    hash_list_traverse(ts->list, (hash_list_traverse_cb)&tcp_server_send_connection_meta, &client_fd);
  }
}
int tcp_server_init(tcp_server *ts, const char *addr, int port, int max_connections)
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
    ts->list = hash_list_alloc(4096);
    return 0;
  }
}
int tcp_server_run(tcp_server *ts)
{
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
          connection_meta *meta = NULL;
          fetch_client_address(ts->connections_events[i].data.fd, (char *)&addr, 32);
          if (tmp.kind == connection_in)
          {
            meta = connection_meta_alloc(tmp.kind, (char *)&tmp.addr);
            hash_list_insert(ts->list, (char *)&tmp.addr, (void *)meta);
            tcp_server_push_to_client(ts, ts->connections_events[i].data.fd, meta);
            fprintf(stdout, "%s connected\n", (char *)&addr);
          }
          else
          {
            meta = hash_list_remove(ts->list, (char *)&tmp.addr);
            tcp_server_push_to_client(ts, ts->connections_events[i].data.fd, meta);
            epoll_ctl(ts->efd, EPOLL_CTL_DEL, ts->connections_events[i].data.fd, &ts->event);
            meta->kind = connection_out;
            fprintf(stdout, "%s leave\n", (char *)&addr);
          }
        }
      }
    }
  }
}
int main(int argc, char *argv[])
{
  tcp_server ts;
  memset(&ts, 0, sizeof(ts));
  tcp_server_init(&ts, argv[1], atoi(argv[2]), 1024);
  tcp_server_run(&ts);

  return 0;
}
