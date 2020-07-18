/*************************************************************************
  > File Name: hash_list.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Friday, July 03, 2020 PM02:24:10 HKT
 ************************************************************************/

#include "hash_list.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct hash_list_node_t {
  struct hash_list_node_t *next;
  void *data;
} hash_list_node;
static hash_list_node *hash_list_node_alloc(void *data) {
  hash_list_node *n = (hash_list_node *)calloc(1, sizeof(*n));
  n->next = NULL;
  n->data = data;
  return n;
}
static void hash_list_node_free(hash_list_node *node) {
  free(node->data);
  free(node);
  node = NULL;
}
int hash_list_insert(hash_list *list, const char *key, void *item) {
  uint64_t h = hash_gfs(key, strlen(key));
  uint32_t index = h % list->max_size;
  hash_list_node *node = hash_list_node_alloc(item);
  if (list->arrays[index] == NULL) {
    list->arrays[index] = node;
    return 0;
  }
  hash_list_node *cur = list->arrays[index];
  hash_list_node *prev = NULL;
  while (cur != NULL) {
    cur = cur->next;
    prev = cur;
  }
  prev->next = node;
  return 0;
}
hash_list *hash_list_alloc(size_t max_size) {}
void *hash_list_remove(hash_list *list, const char *key) {
  uint64_t h = hash_gfs(key, strlen(key));
  uint32_t index = h % list->max_size;
  if (list->arrays[index] == NULL) {

    return -1;
  }
  hash_list_node *cur = list->arrays[index];
  hash_list_node *prev = NULL;
  while (cur != NULL) {

    hash_list_node *next = cur->next;

    char *v = (char *)cur->data + sizeof(message);
    if (strncmp(v, key, strlen(v)) == 0) {
      break;
    } else {
      prev = cur;
    }
    cur = next;
  }
  if (prev != NULL) {
    prev->next = cur->next;
  }
  if (cur != NULL) {
    hash_list_node_free(cur);
    cur = NULL;
  }
  return 0;
}
void hash_list_travel(hash_list *list, void *msg, hash_list_travel_cb cb) {}
void hash_list_free(hash_list *list) {}