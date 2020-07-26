/*************************************************************************
  > File Name: message.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Sun 26 Jul 2020 03:09:42 PM CST
 ************************************************************************/

#ifndef _MESSAGE_H
#define _MESSAGE_H
#include <stdint.h>
typedef enum connection_message_type {
   connection_in=0,
   connection_out
}connection_message_type;
struct connection_message {
   uint8_t kind;
   char uuid[16];
};
struct connection_message *connection_message_alloc(int kind,char *uid);
void connection_message_free(struct connection_message *cm);
#endif
