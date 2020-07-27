/*************************************************************************
  > File Name: hash_list.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Friday, July 03, 2020 PM02:24:10 HKT
 ************************************************************************/

#include "hash_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DM_DELTA 0x9E3779B9
#define DM_FULLROUNDS 10 /* 32 is overkill, 16 is strong crypto */
#define DM_PARTROUNDS 6  /* 6 gets complete mixing */
typedef struct hash_list_node_t
{
  hash_list_node *next;
  char *key;
  void *data;
} hash_list_node;
static uint32_t __pad(int len)
{
  uint32_t pad = 0;

  pad = (uint32_t)len | ((uint32_t)len << 8);
  pad |= pad << 16;

  return pad;
}
static int dm_round(int rounds, uint32_t *array, uint32_t *h0, uint32_t *h1)
{
  uint32_t sum = 0;
  int n = 0;
  uint32_t b0 = 0;
  uint32_t b1 = 0;

  b0 = *h0;
  b1 = *h1;

  n = rounds;

  do
  {
    sum += DM_DELTA;
    b0 += ((b1 << 4) + array[0]) ^ (b1 + sum) ^ ((b1 >> 5) + array[1]);
    b1 += ((b0 << 4) + array[2]) ^ (b0 + sum) ^ ((b0 >> 5) + array[3]);
  } while (--n);

  *h0 += b0;
  *h1 += b1;

  return 0;
}
uint64_t hash_gfs(const char *msg, int len)
{
  uint32_t h0 = 0x9464a485;
  uint32_t h1 = 0x542e1a94;
  uint32_t array[4];
  uint32_t pad = 0;
  int i = 0;
  int j = 0;
  int full_quads = 0;
  int full_words = 0;
  int full_bytes = 0;
  uint32_t *intmsg = NULL;
  int word = 0;

  intmsg = (uint32_t *)msg;
  pad = __pad(len);

  full_bytes = len;
  full_words = len / 4;
  full_quads = len / 16;

  for (i = 0; i < full_quads; i++)
  {
    for (j = 0; j < 4; j++)
    {
      word = *intmsg;
      array[j] = word;
      intmsg++;
      full_words--;
      full_bytes -= 4;
    }
    dm_round(DM_PARTROUNDS, &array[0], &h0, &h1);
  }

  for (j = 0; j < 4; j++)
  {
    if (full_words)
    {
      word = *intmsg;
      array[j] = word;
      intmsg++;
      full_words--;
      full_bytes -= 4;
    }
    else
    {
      array[j] = pad;
      while (full_bytes)
      {
        array[j] <<= 8;
        array[j] |= msg[len - full_bytes];
        full_bytes--;
      }
    }
  }
  dm_round(DM_FULLROUNDS, &array[0], &h0, &h1);

  return (uint64_t)(h0 ^ h1);
}
static hash_list_node *hash_list_node_alloc(void *data)
{
  hash_list_node *n = (hash_list_node *)calloc(1, sizeof(*n));
  n->next = NULL;
  n->data = data;
  return n;
}
static void hash_list_node_free(hash_list_node *node)
{
  free(node->data);
  free(node);
  node = NULL;
}
int hash_list_insert(hash_list *list, const char *key, void *item)
{
  uint64_t h = hash_gfs(key, strlen(key));
  uint32_t index = h % list->max_size;
  hash_list_node *node = hash_list_node_alloc(item);
  if (list->arrays[index] == NULL)
  {
    list->arrays[index] = node;
    return 0;
  }
  hash_list_node *cur = list->arrays[index];
  hash_list_node *prev = NULL;
  while (cur != NULL)
  {
    cur = cur->next;
    prev = cur;
  }
  prev->next = node;
  return 0;
}
hash_list *hash_list_alloc(size_t max_size)
{
  hash_list *lt = (hash_list *)calloc(1, sizeof(*lt));
  lt->max_size = max_size;
  lt->arrays = (void **)calloc(max_size, sizeof(void *));
  return lt;
}
int hash_list_remove(hash_list *list, const char *key)
{
  uint64_t h = hash_gfs(key, strlen(key));
  uint32_t index = h % list->max_size;
  if (list->arrays[index] == NULL)
  {

    return -1;
  }
  hash_list_node *cur = list->arrays[index];
  hash_list_node *prev = NULL;
  while (cur != NULL)
  {

    hash_list_node *next = cur->next;
    char *v = (char *)cur->key;
    if (strncmp(v, key, strlen(v)) == 0)
    {
      break;
    }
    else
    {
      prev = cur;
    }
    cur = next;
  }
  if (prev != NULL)
  {
    prev->next = cur->next;
  }
  if (cur != NULL)
  {
    cur = NULL;
  }
  return 0;
}
void hash_list_traverse(hash_list *list,hash_list_traverse_cb cb)
{
  if(list!=NULL && cb!=NULL)
  {
    for(size_t i=0;i<list->max_size;i++ )
    {
      hash_list_node *cur = list->arrays[i];
      while(cur!=NULL) {
        void *data = cur->data;
        cb(data);
        cur = cur->next;
      }
    }
  }
}
void hash_list_free(hash_list *list) {}