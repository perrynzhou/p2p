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

int main(int argc, char *argv[])
{
  int port = atoi(argv[2]);
  char *addr = argv[1];
  int client_fd = init_tcp_client(addr, port);
  struct hash_list *list = hash_list_alloc(4096);
  char buffer[4096] = {'\0'};
  char *ptr = (char *)&buffer;
  init_uuid(ptr, 4096);
  struct connection_message *cm = connection_message_alloc(connection_in, ptr);
  if (send(client_fd, cm, sizeof(*cm), 0) >0)
  {
    fprintf(stdout, "handshark success\n");
  }
  while (1)
  {
    sleep(1);
  }
}