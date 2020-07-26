/*************************************************************************
    > File Name: tcp_server.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Sun 26 Jul 2020 07:44:00 PM CST
 ************************************************************************/

#ifndef _TCP_SERVER_H
#define _TCP_SERVER_H
#include <sys/epoll.h>
struct tcpServer {
  int efd;
  struct epoll_event *connections_events;
  struct epoll_event event;
  uint32_t max_connections;
  char *addr;
  int port;
};

int tcp_server_init(struct tcpServer *ts,const char *addr,int port,int max_connections);
int tcp_server_start(struct tcpServer *ts);
void tcp_server_deinit(struct tcpServer *ts);
#endif
