/*************************************************************************
    > File Name: p2p_client.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Tuesday, July 28, 2020 PM04:59:41
 ************************************************************************/

#ifndef _P2P_CLIENT_H
#define _P2P_CLIENT_H
#include "hash_list.h"
#include "message.h"
typedef struct p2p_client_t
{
  char *name;
  hash_list *list;
  int tcp_sock;
  int udp_sock;
  pthread_t input_thd;
  pthread_t read_thd;
  connection_meta meta;
} p2p_client;

p2p_client *p2p_client_alloc(const char *name, int sockfd, int hash_list_max_size);
int p2p_client_shake_hands(p2p_client *tc);
int p2p_client_run(p2p_client *tc);
void p2p_client_free(p2p_client *tc);

#endif
