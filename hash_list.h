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
struct hash_list
{
  size_t max_size;
  void **arrays;
  pthread_mutex_t mutx;
} ;
struct hash_list *hash_list_alloc(size_t max_size);
int hash_list_insert(struct hash_list *list, const char *key, void *item);
int hash_list_remove(struct hash_list *list, const char *key);
void hash_list_free(struct hash_list *list);
#endif
