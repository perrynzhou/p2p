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
inline static int tcp_server_send_connection_meta(void *ctx, void *data)
{
  connection_meta *meta = (connection_meta *)data;
  int *fd = (int *)ctx;
  if (send(*fd, meta, sizeof(*meta), MSG_DONTWAIT) > 0)
  {
    fprintf(stderr, "push %s to client success\n", meta->addr);
    return 0;
  }
  return -1;
}
inline static int tcp_server_cache_clients(void *ctx, void *data)
{
  connection_meta *meta = (connection_meta *)data;
  hash_list *list = (hash_list *)ctx;
  hash_list_insert(list, meta->addr, meta);
  return 0;
}
static void tcp_server_push_in_to_client(tcp_server *ts, connection_meta *meta, int index)
{
  hash_list *cache = ts->caches[index].client_list;
  if (cache->cur_size == 0)
  {
    hash_list_traverse(ts->list, tcp_server_cache_clients, cache);
    hash_list_traverse(cache, tcp_server_send_connection_meta, &ts->caches[index].fd);
  }
  for (int i = ts->sfd + 1; i < ts->cache_size; i++)
  {
    if (cache != NULL && index != i && ts->caches[i].fd != -1)
    {
      hash_list *cache = ts->caches[i].client_list;
      if (cache != NULL && cache->cur_size > 0)
      {
        tcp_server_send_connection_meta(&ts->caches[i].fd, meta);
        hash_list_insert(cache, meta->addr, meta);
      }
    }
  }
}
static int tcp_server_push_out_to_client(tcp_server *ts, connection_meta *meta, int index)
{
  hash_list *cache = ts->caches[index].client_list;
  meta->kind = connection_out;
  tcp_server_send_connection_meta(&ts->caches[index].fd, meta);
  int curr_fd = ts->caches[index].fd;
  ts->caches[index].fd = -1;
  hash_list_free(ts->caches[index].client_list, NULL);
  ts->caches[index].client_list = NULL;
  for (int i = ts->efd + 1; i < ts->cache_size; i++)
  {
    tcp_client_cache_item *item = &ts->caches[i];
    cache = ts->caches[i].client_list;
    if (cache != NULL && cache->cur_size > 0 && item->fd != -1 && item->fd != curr_fd)
    {
      hash_list_remove(cache, (char *)&meta->addr);
      tcp_server_send_connection_meta(&ts->caches[i].fd, meta);
    }
  }
}
int tcp_server_init(tcp_server *ts, const char *addr, int port, int max_connections)
{
  if (ts != NULL)
  {
    ts->max_connections = max_connections;
    int sfd = init_tcp_socket(addr, port, 0);
    if (sfd == -1)
    {
      return -1;
    }
    ts->efd = epoll_create(max_connections);
    ts->cache_size = 65535;
    ts->caches = (tcp_client_cache_item *)calloc(ts->cache_size, sizeof(tcp_client_cache_item));
    for (size_t i = 0; i < max_connections; i++)
    {
      ts->caches[i].fd = -1;
      ts->caches[i].client_list = NULL;
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
    ts->list = hash_list_alloc(max_connections);
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
      }
      else
      {
        int clientfd = ts->connections_events[i].data.fd;
        connection_meta tmp;

        ssize_t count = recv(ts->connections_events[i].data.fd, &tmp, sizeof(tmp), 0);
        if (count > 0)
        {
          char addr[32] = {'\0'};
          connection_meta *meta = NULL;
          fetch_ip_address_from_fd(ts->connections_events[i].data.fd, (char *)&addr, 32);
          if (tmp.kind == connection_in)
          {
            if (ts->caches[clientfd].fd == -1)
            {
              ts->caches[clientfd].fd = clientfd;
              ts->caches[clientfd].client_list = hash_list_alloc(ts->max_connections);
            }
            meta = connection_meta_alloc(tmp.kind, (char *)&tmp.addr);
            hash_list_insert(ts->list, (char *)tmp.addr, (void *)meta);
            tcp_server_push_in_to_client(ts, meta, clientfd);
            fprintf(stdout, "fd=%d %s connected\n", clientfd, (char *)&addr);
          }
          else
          {
            meta = hash_list_remove(ts->list, (char *)&tmp.addr);
            tcp_server_push_out_to_client(ts, meta, clientfd);
            epoll_ctl(ts->efd, EPOLL_CTL_DEL, ts->connections_events[i].data.fd, &ts->event);
            connection_meta_free(meta);
            fprintf(stdout, "fd=%d %s disconnected\n", clientfd, (char *)&addr);
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
  char local_addr[128] = {'\0'};
  fetch_ip_address_from_localhost((char *)&local_addr, 128);
  tcp_server_init(&ts, (char *)&local_addr, atoi(argv[1]), 1024);
  tcp_server_run(&ts);
  return 0;
}
