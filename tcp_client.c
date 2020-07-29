/*************************************************************************
  > File Name: tcp_client.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Friday, July 03, 2020 PM02:24:30 HKT
 ************************************************************************/

#include "tcp_client.h"
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

int tcp_client_print_connection_meta(void *ctx, void *data)
{
  connection_meta *meta = (connection_meta *)data;
  fprintf(stdout, " %s\n", meta->addr);
}
int tcp_client_handle_list_cmd(tcp_client *client)
{
  hash_list_traverse(client->list, (hash_list_traverse_cb)tcp_client_print_connection_meta, NULL);
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
static void *tcp_client_cache(tcp_client *client)
{
  connection_meta *meta = NULL;
  int done = 1;
  while (1)
  {
    ssize_t count = recv(client->tcp_sock, &client->meta, sizeof(client->meta), 0);
    if (count > 0)
    {
      switch (client->meta.kind)
      {
      case connection_in:
        meta = connection_meta_alloc(client->meta.kind, (char *)&client->meta.addr);
        hash_list_insert(client->list, (char *)&client->meta.addr, meta);
        break;
      case connection_out:
        meta = hash_list_remove(client->list, (char *)&client->meta.addr);
        connection_meta_free(meta);
        meta = NULL;
        break;
      default:
        break;
      }
    }
  }
  pthread_detach(pthread_self());
  pthread_exit(NULL);
}
tcp_client *tcp_client_alloc(const char *name, int sockfd, int hash_list_max_size)
{
  tcp_client *client = (tcp_client *)calloc(1, sizeof(tcp_client));
  client->list = hash_list_alloc(hash_list_max_size);
  client->tcp_sock = sockfd;
  if (name != NULL)
  {
    client->name = strdup(name);
  }
  return client;
}
int tcp_client_send(tcp_client *client, int conection_type)
{
  connection_meta_reset(&client->meta, conection_type, client->name);
  int ret = -1;
  if ((ret = send(client->tcp_sock, &client->meta, sizeof(client->meta), 0)) > 0)
  {
    if (conection_type == connection_in)
    {
      fprintf(stdout, "::connected to tcp server success::\n");
    }
    else
    {
      fprintf(stdout, "::disconnected from tcp server success::\n");
    }
    return 0;
  }
  return -1;
}
static void tcp_client_gen_name(tcp_client *client, int port)
{
  char name[128] = {'\0'};
  fetch_ip_address_from_localhost((char *)&name, 128);
  size_t name_len = strlen((char *)&name);
  snprintf((char *)&name + name_len, 128 - name_len, ":%d", port);
  client->name = strdup((char *)&name);
}
inline static void tcp_client_notify(tcp_client *client)
{
  pthread_create(&client->tid, NULL, (void *)&tcp_client_cache, client);
}
static void tcp_client_handle_input(tcp_client *client, char **buf)
{
  *buf = (char *)calloc(4096, sizeof(char));
  char *input = *buf;
  fprintf(stdout, "enter->");
  while (fgets(input, 4096, stdin) != NULL)
  {
    size_t len = strlen(input);
    if (strncmp(input, "list\n", 5) == 0)
    {
      tcp_client_handle_list_cmd(client);
    }
    else if (strncmp(input, "quit\n", 5) == 0)
    {
      tcp_client_send(client, connection_out);
      break;
    }
    fprintf(stdout, "enter->");
  }
  if (*buf != NULL)
  {
    free(*buf);
    *buf = NULL;
  }
}
void tcp_client_free(tcp_client *client)
{
  if (client != NULL)
  {
    if (client->list != NULL)
    {
      hash_list_free(client->list, (hash_list_data_free_cb)connection_meta_free);
      client->list = NULL;
    }
    if (client->name != NULL)
    {
      free(client->name);
      client->name = NULL;
    }
    if (client->tcp_sock != -1)
    {
      close(client->tcp_sock);
    }
    free(client);
    client = NULL;
  }
}
int main(int argc, char *argv[])
{
  char *remote_addr = argv[1];
  int remote_port = atoi(argv[2]);
  int sock = init_tcp_client(remote_addr, remote_port);
  int local_port = atoi(argv[3]);
  char name[128] = {'\0'};
  char *buffer = NULL;
  tcp_client *client = tcp_client_alloc(NULL, sock, 4096);
  tcp_client_gen_name(client, local_port);
  tcp_client_send(client, connection_in);
  tcp_client_notify(client);
  tcp_client_handle_input(client, &buffer);
  return 0;
}