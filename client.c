/*************************************************************************
  > File Name: client.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Friday, July 03, 2020 PM02:24:30 HKT
 ************************************************************************/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "hash_list.h"
#include "utils.h"
typedef struct client_peer_t
{
  int tcp_fd;
  hash_list *list;
} client_peer;
static void del_client_addr_cb(void *addr, void *data);
static void sync_client_addr(hash_list *list, char *buffer)
{
  message *m = (message *)p_buf;
  char buffer[4096] = {'\0'};
  snprintf((char *)&buffer, 4096, "%s", buffer + sizeof(message));
  if (m->type == REGISTER)
  {
    char *client_info = strdup((char *)&buffer);
    hash_list_insert(list, buffer + sizeof(message), client_info);
    return;
  }
  hash_list_travel(list, vbuffer + sizeof(message), (hash_list_travel_cb)&del_client_addr_cb);
}
//new thread to update sync
void *sync_client_peer_thread_func(void *arg)
{
  client_peer *cp = (client_peer *)arg;
  while (1)
  {
    char buf[4096] = {'\0'};
    char *p_buf = (char *)&buf;
    ssize_t sz = recv(cp->tcp_fd, p_buf, 0);
    if (sz > 0)
    {
      update_client_addr(cp->list, p_buf);
    }
  }
}
static int init_udp_socket(const char *addr, int port)
{
  int fd = socket(PF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in serveraddr;
  serveraddr.sin_family = AF_INET;
  inet_pton(AF_INET, addr, &serveraddr.sin_addr);
  serveraddr.sin_port = htons(port);
  bind(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
  return fd;
}
static int init_tcp_client(const char *addr, int port)
{
  struct sockaddr_in srv_addr;
  printf("break!");
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  printf("We get the sockfd~\n");
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(port);
  srv_addr.sin_addr.s_addr = inet_addr(addr);
  bzero(&(srv_addr.sin_zero), 8);
  return sockfd;
}
void list_peer_client(client_peer *peer)
{
  hash_list_travel(peer, NULL, NULL;)
}
int main(int argc, char *argv[])
{
  int port = atoi(argv[2]);
  char *addr = argv[1];
  int tcp_fd = init_tcp_client(addr, port);
  client_peer *peer = calloc(1, sizeof(client_peer));
  peer->list = hash_list_alloc(4096);
  peer->tcp_fd = tcp_fd;
  pthread_t tid;
  pthread_create(&tid, NULL, &sync_client_peer_thread_func, peer);
  char line_buffer[4096] = {'\0'};
  char *ptr = (char *)&line_buffer;
  while (fgets(ptr, 4096, stdin) != NULL)
  {
    if (strncmp(ptr, "list", 4) == 0)
    {
      list_peer_client(peer);
    }
  }
  pthread_join(tid, NULL);
}