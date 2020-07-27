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
connection_meta *connection_metae_alloc(int kind,const char *addr)
{
  connection_meta *cm = calloc(1,sizeof(connection_meta));
  cm->kind=kind;
  snprintf((char *)cm->addr,128,"%s",addr);
  return cm;
}
void connection_meta_free(connection_meta *cm)
{
  if(cm!=NULL) {
    free(cm);
    cm = NULL;
  }
}