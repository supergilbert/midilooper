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

  list->len = 0;
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
      list->tail = new_node;
      /* TODO: Atomic assigment */
      list->head = new_node;
    }
  else
    {
      new_node->addr = addr;
      new_node->next = list->head;
      new_node->prev = NULL;
      list->head->prev = new_node;
      /* Atomic assigment */
      list->head = new_node;
    }
  (list->len)++;
}

void _del_list_node(list_t *list, node_t *node, free_list_func func)
{
  if (list->head == node)
    {
      if (list->tail == node)
        {
          list->head = NULL;
          list->tail = NULL;
        }
      else
        {
          list->head = node->next;
          node->next->prev = NULL;
        }
    }
  else if (list->tail == node)
    {
      node->prev->next = NULL;
      list->tail = node->prev;
    }
  else
    {
      node->prev->next = node->next;
      node->next->prev = node->prev;
    }
  (list->len)--;
  func(node->addr);
  free(node);
}

void push_to_list_tail(list_t *list, void *addr)
{
  node_t *new_node = myalloc(sizeof (node_t));

  new_node->addr = addr;
  new_node->next = NULL;
  if (list->tail == NULL)
    {
      new_node->prev = NULL;
      list->tail = new_node;
      list->head = new_node;
    }
  else
    {
      new_node->prev = list->tail;
      list->tail = new_node;
      new_node->prev->next = new_node;
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
      iterator->list->tail = node;
      /* Atomic assigment */
      iterator->list->head = node;
    }
  else
    {
      /* iterator->node->prev = node; */
      node->next = iterator->node;
      if (iterator->node->prev == NULL)
        {
          node->prev = NULL;
          iterator->node->prev = node;
          /* Atomic assigment */
          iterator->list->head = node;
        }
      else
        {
          node->prev = iterator->node->prev;
          /* Atomic assigment */
          iterator->node->prev->next = node;
          iterator->node->prev = node;
        }
    }
  (iterator->list->len)++;
  return;
}

void iter_node_del(list_iterator_t *iterator, free_list_func func)
{
  list_t        *list = iterator->list;
  node_t        *node = NULL;

  if (list->head)
    {
      if (list->head == list->tail)
        {
          _del_list_node(iterator->list, iterator->node, func);
          iterator->node = NULL;
        }
      else
        {
          if (iterator->node->next == NULL)
            {
              _del_list_node(iterator->list, iterator->node, func);
              iterator->node = list->tail;
            }
          else
            {
              node = iterator->node->next;
              _del_list_node(iterator->list, iterator->node, func);
              iterator->node = node;
            }
        }
    }
}
