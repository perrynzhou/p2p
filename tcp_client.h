/*************************************************************************
    > File Name: tcp_client.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Tuesday, July 28, 2020 PM04:59:41
 ************************************************************************/

#ifndef _TCP_CLIENT_H
#define _TCP_CLIENT_H
#include "hash_list.h"
#include "message.h"
typedef struct tcp_client_t
{
  char *name;
  hash_list *list;
  int tcp_sock;
  int udp_sock;
  pthread_t tid;
  pthread_t wid;
  connection_meta meta;
} tcp_client;

tcp_client *tcp_client_alloc(const char *name, int sockfd, int hash_list_max_size);
int tcp_client_shake_hands(tcp_client *tc);
int tcp_client_run(tcp_client *tc);
void tcp_client_free(tcp_client *tc);

#endif
