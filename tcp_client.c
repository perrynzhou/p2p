/*************************************************************************
  > File Name: tcp_client.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Friday, July 03, 2020 PM02:24:30 HKT
 ************************************************************************/

#include "message.h"
#include "hash_list.h"
#include "util.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
typedef struct udp_server_t
{
  int sock;
  int pipe[2];
  struct sockaddr_in udp_addr;
} udp_server;
typedef struct tcp_client_t
{
  char *local_addr;
  hash_list *lt;
  int tcp_sock;
} tcp_client;
int tcp_client_print_connection_meta(void *ctx, void *data)
{
  connection_meta *meta = (connection_meta *)data;
  fprintf(stdout, "%s\n", meta->addr);
}
int tcp_client_handle_list_cmd(tcp_client *tc)
{
  hash_list_traverse(tc->lt, (hash_list_traverse_cb)tcp_client_print_connection_meta, NULL);
}
static int tcp_client_init_chat_env(udp_server *us, const char *udp_remote_addr, int udp_remote_port)
{

  us->sock = init_udp_socket(udp_remote_addr, udp_remote_port);
  bzero(&us->udp_addr, sizeof(us->udp_addr));
  us->udp_addr.sin_family = AF_INET;
  us->udp_addr.sin_port = htons(udp_remote_port);
  us->udp_addr.sin_addr.s_addr = inet_addr(udp_remote_addr);
  return 0;
}
static void *tcp_client_cache(tcp_client *tc)
{
  connection_meta cm;
  connection_meta *new_con = NULL;
  while (1)
  {
    ssize_t count = recv(tc->tcp_sock, &cm, sizeof(cm), 0);
    if (count > 0)
    {
      switch (cm.kind)
      {
      case connection_in:
        new_con = connection_meta_alloc(cm.kind, (char *)&cm.addr);
        hash_list_insert(tc->lt, (char *)&cm.addr, new_con);
      case connection_out:
        hash_list_remove(tc->lt, (char *)&cm.addr);
      default:
        break;
      }
    }
  }
}
static tcp_client *tcp_client_alloc(const char *tcp_remote_addr, int tcp_remote_port, const char *local_addr, int local_port)
{
  int client_fd = init_tcp_client(tcp_remote_addr, tcp_remote_port);
  char buffer[4096] = {'\0'};
  char *ptr = (char *)&buffer;
  snprintf(ptr, 4096, "%s:%d", local_addr, local_port);
  connection_meta *cm = connection_meta_alloc(connection_in, ptr);
  if (send(client_fd, cm, sizeof(*cm), 0) > 0)
  {
    fprintf(stdout, "handshark success\n");
  }
  tcp_client *tc = (tcp_client *)calloc(1, sizeof(tcp_client));
  tc->local_addr = strdup(ptr);
  tc->lt = hash_list_alloc(4096);
  tc->tcp_sock = client_fd;
  return tc;
}

int main(int argc, char *argv[])
{
  int srv_tcp_port = atoi(argv[2]);
  char *srv_tcp_addr = argv[1];
  char *local_addr = argv[3];
  int local_port = atoi(argv[4]);
  tcp_client *tc = tcp_client_alloc(srv_tcp_addr, srv_tcp_port, local_addr, local_port);
  pthread_t tid;
  pthread_create(&tid, NULL, (void *)&tcp_client_cache, tc);
  char *buffer = (char *)calloc(4096, sizeof(char));
  while (fgets(buffer, 4096, stdin) != NULL)
  {
    fprintf(stdout, "enter->");
    size_t len = strlen(buffer);
    buffer[len] = '\0';
    if (strncmp(buffer, "list", 4) == 0)
    {
      tcp_client_handle_list_cmd(tc);
    }
  }
  pthread_join(tid, NULL);
  return 0;
}