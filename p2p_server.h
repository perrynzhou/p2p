/*************************************************************************
    > File Name: p2p_server.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Sun 26 Jul 2020 07:44:00 PM CST
 ************************************************************************/

#ifndef _P2P_SERVER_H
#define _P2P_SERVER_H
#include "hash_list.h"
#include <sys/epoll.h>
typedef struct p2p_client_cache_item_t {
  int fd;
  hash_list *client_list;
}p2p_client_cache_item;
typedef struct p2p_server_t
{
  int efd;
  int sfd;
  uint32_t max_connections;
  struct epoll_event event;
  struct epoll_event *connections_events;
  hash_list *list;
  uint32_t cache_size;
  p2p_client_cache_item *caches;
} p2p_server;
int p2p_server_init(p2p_server *ts, const char *addr, int port, int max_connections);
int p2p_server_run(p2p_server *ts);
#endif
