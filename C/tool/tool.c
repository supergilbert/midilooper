#include <stdlib.h>
#include <strings.h>

#include "debug_tool/debug_tool.h"
#include "tool/tool.h"

void *myalloc(size_t size)
{
  void *addr = malloc(size);

  bzero(addr, size);
  return addr;
}

void free_list_node(list_t *list, free_list_func func)
{
  node_t *node = list->head;
  node_t *next = NULL;
  list->head = NULL;
  list->tail = NULL;
  while (node)
    {
      next = NEXT_NODE(node);
      func(node->addr);
      free(node);
      node = next;
    }
}

void foreach_list_node(list_t *list, list_func func, void *args)
{
  node_t *node = list->head;

  while (node)
    {
      func(node->addr, args);
      node = NEXT_NODE(node);
    }
}

void push_to_list(list_t *list, void *addr)
{
  node_t *new_node = myalloc(sizeof (node_t));

  if (list->head == NULL)
    {
      new_node->addr = addr;
      new_node->next = NULL;
      new_node->prev = NULL;
      list->head = new_node;
      list->tail = new_node;
    }
  else
    {
      new_node->addr = addr;
      new_node->next = list->head;
      new_node->prev = NULL;
      list->head->prev = new_node;
      list->head = new_node;
    }
  (list->len)++;
}

void push_to_list_tail(list_t *list, void *addr)
{
  node_t *new_node = myalloc(sizeof (node_t));

  if (list->tail == NULL)
    {
      new_node->addr = addr;
      new_node->next = NULL;
      new_node->prev = NULL;
      list->head = new_node;
      list->tail = new_node;
    }
  else
    {
      new_node->addr = addr;
      new_node->prev = list->tail;
      new_node->next = NULL;
      list->tail->next = new_node;
      list->tail = new_node;
    }
  (list->len)++;
}

void iter_init(list_iterator_t *iterator, list_t *list)
{
  iterator->node = list->head;
  iterator->list = list;
}

void iter_push_before(list_iterator_t *iterator, void *addr)
{
  node_t *node = myalloc(sizeof (node_t));

  node->addr = addr;
  if (iterator->node == NULL)
    {
      node->prev = NULL;
      node->next = NULL;
      iterator->list->head = node;
      iterator->list->tail = node;
    }
  else
    {
      /* iterator->node->prev = node; */
      node->next = iterator->node;
      if (iterator->node->prev == NULL)
        {
          node->prev = NULL;
          iterator->node->prev = node;
          iterator->list->head = node;
        }
      else
        {
          node->prev = iterator->node->prev;
          iterator->node->prev->next = node;
          iterator->node->prev = node;
        }
    }
  (iterator->list->len)++;
  return;
}
