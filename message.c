/*************************************************************************
  > File Name: message.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Sun 26 Jul 2020 03:09:47 PM CST
 ************************************************************************/

#include "message.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct connection_message *connection_message_alloc(int kind,char *uuid)
{
  struct connection_message *cm = calloc(1,sizeof(struct connection_message));
  strncpy((char *)&cm->uuid,uuid,strlen(uuid));
  cm->kind=kind;
  return cm;
}
void connection_message_free(struct connection_message *cm)
{
  if(cm!=NULL) {
    free(cm);
    cm = NULL;
  }
}