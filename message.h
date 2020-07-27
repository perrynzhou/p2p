/*************************************************************************
  > File Name: message.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Sun 26 Jul 2020 03:09:42 PM CST
 ************************************************************************/

#ifndef _MESSAGE_H
#define _MESSAGE_H
#include <stdint.h>
typedef enum message_type_t {
   connection_in=0,
   connection_out
}message_type;
typedef struct connection_meta_t {
   uint8_t kind;
   char addr[128];
}connection_meta;

typedef struct message_t {
    uint8_t kind;
    uint32_t len;
    char   data[0];
}message;
connection_meta *connection_meta_alloc(int kind,const char *addr);
void connection_meta_free(connection_meta *cm);
#endif
