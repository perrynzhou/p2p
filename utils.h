/*************************************************************************
  > File Name: utils.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Friday, July 03, 2020 PM02:25:20 HKT
 ************************************************************************/

#ifndef _UTILS_H
#define _UTILS_H
#include <stdint.h>
typedef enum message_type_t {
    REGISTER=0,
    LOGGIN,
    LOGOUT,
    UNREGISTER,
    MESSAGE
}message_type;
typedef struct message_t {
  uint8_t type;
  uint32_t   len;
  char *body;
}message;
#endif
