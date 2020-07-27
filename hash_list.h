/*************************************************************************
  > File Name: hash_list.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Friday, July 03, 2020 PM02:24:02 HKT
 ************************************************************************/

#ifndef _HASH_LIST_H
#define _HASH_LIST_H
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
typedef int (*hash_list_traverse_cb)(void *);
typedef struct hash_list_t
{
  size_t max_size;
  void **arrays;
  pthread_mutex_t mutx;
} hash_list;
hash_list *hash_list_alloc(size_t max_size);
int hash_list_insert(hash_list *list, const char *key, void *item);
int hash_list_remove(hash_list *list, const char *key);
void hash_list_traverse(hash_list *list,hash_list_traverse_cb cb);
void hash_list_free(hash_list *list);
#endif
