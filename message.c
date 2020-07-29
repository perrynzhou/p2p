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
void connection_meta_reset(connection_meta *meta, int kind, const char *addr)
{
  meta->kind = kind;
  if(addr !=NULL) {
      snprintf((char *)meta->addr, 128, "%s", addr);
  }
}
connection_meta *connection_meta_alloc(int kind, const char *addr)
{
  connection_meta *meta = calloc(1, sizeof(connection_meta));
  connection_meta_reset(meta, kind, addr);
  return meta;
}
void connection_meta_free(connection_meta *meta)
{
  if (meta != NULL)
  {
    free(meta);
    meta = NULL;
  }
}