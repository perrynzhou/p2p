/*************************************************************************
    > File Name: tcp_server.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Sun 26 Jul 2020 07:44:00 PM CST
 ************************************************************************/

#ifndef _TCP_SERVER_H
#define _TCP_SERVER_H
#include <sys/epoll.h>
typedef struct tcp_server_t
{
  int efd;
  int sfd;
  uint32_t max_connections;
  struct epoll_event event;
  struct epoll_event *connections_events;
  hash_list *list;
}tcp_server;
int tcp_server_init( tcp_server *ts, const char *addr, int port, int max_connections);
int tcp_server_run( tcp_server *ts);
#endif
