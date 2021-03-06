/*************************************************************************
  > File Name: p2p_client.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Friday, July 03, 2020 PM02:24:30 HKT
 ************************************************************************/

#include "p2p_client.h"
#include "message.h"
#include "hash_list.h"
#include "util.h"
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
int p2p_client_send_message(void *message_ptr, void *key_ptr)
{
  char *key = key_ptr;
  char *message = message_ptr;
  char buffer[128] = {'\0'};
  snprintf((char *)buffer, 128, "%s", key);
  char *string = (char *)&buffer;
  char *addr = strsep(&string, ":");
  int port = atoi(strsep(&string, ":"));
  struct sockaddr_in addrsrv;
  bzero(&addrsrv, sizeof(addrsrv));
  int udp_sock = init_udp_socket(addr, port, &addrsrv);
  socklen_t len = sizeof(addrsrv);
  if (sendto(udp_sock, message, strlen(message), 0, (struct sockaddr *)&addrsrv, len) < 0)
  {
    close(udp_sock);
    fprintf(stdout, "send message failed:%s\n", strerror(errno));
  }
  if (udp_sock != -1)
  {
    close(udp_sock);
  }
  return 0;
}

int p2p_client_print_connection_meta(void *ctx, void *data)
{
  connection_meta *meta = (connection_meta *)data;
  fprintf(stdout, " %s\n", meta->addr);
}
int p2p_client_handle_list_cmd(p2p_client *client)
{
  hash_list_traverse(client->list, (hash_list_traverse_cb)p2p_client_print_connection_meta, NULL);
}

static void *p2p_client_cache(p2p_client *client)
{
  connection_meta *meta = NULL;
  while (1)
  {
    ssize_t count = recv(client->tcp_sock, &client->meta, sizeof(client->meta), 0);
    if (count > 0)
    {
      switch (client->meta.kind)
      {
      case connection_in:
        //fprintf(stdout,"::in %s\n",(char *)&client->meta.addr);
        meta = connection_meta_alloc(client->meta.kind, (char *)&client->meta.addr);
        hash_list_insert(client->list, (char *)&client->meta.addr, meta);
        break;
      case connection_out:
        //fprintf(stdout,"::out %s\n",(char *)&client->meta.addr);
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
  return NULL;
}
static void *p2p_client_rev_msg(p2p_client *client)
{
  struct sockaddr_in src_addr;
  socklen_t addrlen = sizeof(src_addr);
  char buffer[4096] = {'\0'};
  char ipaddr[128] = {'\0'};
  while (1)
  {
    size_t count = recvfrom(client->udp_sock, (char *)&buffer, 4096, 0, (struct sockaddr *)&src_addr, &addrlen);
    if (count > 0)
    {
      buffer[count - 1] = '\0';
      fprintf(stderr, "::recv=%s\n", (char *)&buffer);
    }
  }
  return NULL;
}
p2p_client *p2p_client_alloc(const char *name, int sockfd, int hash_list_max_size)
{
  p2p_client *client = (p2p_client *)calloc(1, sizeof(p2p_client));
  client->list = hash_list_alloc(hash_list_max_size);
  client->tcp_sock = sockfd;
  if (name != NULL)
  {
    client->name = strdup(name);
  }
  return client;
}
int p2p_client_send(p2p_client *client, int conection_type)
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
static void p2p_client_gen_name(p2p_client *client, int port)
{
  char name[128] = {'\0'};
  fetch_ip_address_from_localhost((char *)&name, 128);
  size_t name_len = strlen((char *)&name);
  snprintf((char *)&name + name_len, 128 - name_len, ":%d", port);
  client->name = strdup((char *)&name);
}
inline static void p2p_client_notify(p2p_client *client)
{
  pthread_create(&client->input_thd, NULL, (void *)&p2p_client_cache, client);
  pthread_create(&client->input_thd, NULL, (void *)&p2p_client_rev_msg, client);
}
int p2p_client_broadcast(void *message, connection_meta *meta)
{
  char buffer[4096] = {'\0'};
  snprintf((char *)&buffer, 4096, "%s", (char *)meta->addr);
  return p2p_client_send_message(message, (char *)&buffer);
}
int p2p_client_handle_all_cmd(p2p_client *client, void *message)
{
  hash_list_traverse(client->list, (hash_list_traverse_cb)p2p_client_broadcast, message);
}
static void p2p_client_handle_input(p2p_client *client, char **buf)
{
  *buf = (char *)calloc(4096, sizeof(char));
  char *input = *buf;
  fprintf(stdout, "enter->");
  while (fgets(input, 4096, stdin) != NULL)
  {
    size_t len = strlen(input);
    if (strncmp(input, "list\n", 5) == 0)
    {
      p2p_client_handle_list_cmd(client);
    }
    else if (strncmp(input, "quit\n", 5) == 0)
    {
      p2p_client_send(client, connection_out);
      break;
    }
    else
    {
      char buffer[4096] = {'\0'};
      snprintf((char *)&buffer, 4096, "%s", input);
      char *buffer_ptr = (char *)&buffer;
      char *cmd = strsep(&buffer_ptr, " ");
      char *key = strsep(&buffer_ptr, " ");
      char *msg = strsep(&buffer_ptr, " ");
      if (strncmp(cmd, "send", 4) == 0)
      {
        if (strncmp(key, "all", 3) != 0)
        {
          p2p_client_send_message(msg, key);
        }
        else
        {
          p2p_client_handle_all_cmd(client, msg);
        }
      }
    }
    fprintf(stdout, "enter->");
  }
  if (*buf != NULL)
  {
    free(*buf);
    *buf = NULL;
  }
}
void p2p_client_free(p2p_client *client)
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
inline static void p2p_client_init_chat_env(p2p_client *client, int port)
{
  char ip[128] = {'\0'};
  fetch_ip_address_from_localhost((char *)&ip, 128);
  struct sockaddr_in dest_addr;
  client->udp_sock = init_udp_socket((char *)&ip, port, &dest_addr);
}
int main(int argc, char *argv[])
{
  char *remote_addr = argv[1];
  int remote_port = atoi(argv[2]);
  int sock = init_tcp_client(remote_addr, remote_port);
  int local_port = atoi(argv[3]);
  char name[128] = {'\0'};
  char *buffer = NULL;
  p2p_client *client = p2p_client_alloc(NULL, sock, 4096);
  p2p_client_gen_name(client, local_port);
  p2p_client_init_chat_env(client, local_port);
  p2p_client_send(client, connection_in);
  p2p_client_notify(client);
  p2p_client_handle_input(client, &buffer);
  return 0;
}