/* Copyright 2012-2014 Gilbert Romer */

/* This file is part of gmidilooper. */

/* gmidilooper is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* gmidilooper is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with gmidilooper.  If not, see <http://www.gnu.org/licenses/>. */


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
  if (func != NULL)
    {
      while (node)
        {
          next = NEXT_NODE(node);
          func(node->addr);
          free(node);
          node = next;
        }
    }
  else
    while (node)
      {
        next = NEXT_NODE(node);
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

node_t *push_to_list(list_t *list, void *addr)
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
  return new_node;
}

node_t *push_to_list_tail(list_t *list, void *addr)
{
  node_t *new_node = myalloc(sizeof (node_t));

  new_node->addr = addr;
  new_node->next = NULL;
  if (list->tail == NULL)
    {
      new_node->prev = NULL;
      list->tail = new_node;
      /* Atomic assigment */
      list->head = new_node;
    }
  else
    {
      new_node->prev = list->tail;
      list->tail = new_node;
      /* Atomic assigment */
      new_node->prev->next = new_node;
    }
  (list->len)++;
  return new_node;
}

void iter_copy(list_iterator_t *iter_src, list_iterator_t *iter_dst)
{
  iter_dst->node = iter_src->node;
  iter_dst->list = iter_src->list;
}

void iter_init(list_iterator_t *iterator, list_t *list)
{
  iterator->node = list->head;
  iterator->list = list;
}

node_t *iter_push_before(list_iterator_t *iterator, void *addr)
{
  node_t *node = NULL;

  if (iterator->node == NULL)
    push_to_list_tail(iterator->list, addr);
  else
    {
      node = myalloc(sizeof (node_t));
      node->addr = addr;
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
          iterator->node->prev = node; /* (can not be atomic) */
        }
    }
  (iterator->list->len)++;
  return node;
}

node_t *iter_push_after(list_iterator_t *iterator, void *addr)
{
  node_t *node = NULL;

  if (iterator->node == NULL)
    push_to_list_tail(iterator->list, addr);
  else
    {
      node = myalloc(sizeof (node_t));
      node->addr = addr;
      node->prev = iterator->node;
      if (iterator->node->next == NULL)
        {
          node->next = NULL;
          iterator->list->tail = node;
          /* Atomic assigment */
          iterator->node->next = node;
        }
      else
        {
          node->next = iterator->node->next;
          iterator->node->next->prev = node;
          /* Atomic assigment */
          iterator->node->next = node;
        }
    }
  (iterator->list->len)++;
  return node;
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
          msq_assert(node->next != NULL,
                     "node->next can not be null if it is not the tail node");
          node->next->prev = NULL;
          list->head = node->next;
        }
    }
  else
    {
      if (list->tail == node)
        {
          node->prev->next = NULL;
          list->tail = node->prev;
        }
      else
        {
          msq_assert(node->next != NULL,
                     "node->next can not be null if it is not the tail node");
          node->prev->next = node->next;
          node->next->prev = node->prev;
        }
      if (node->prev->prev == NULL)
          list->head = node->prev;
    }
  (list->len)--;
  if (func)
    func(node->addr);
  free(node);
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

bool_t iter_move_to_addr(list_iterator_t *iterator, void *addr)
{
  for (iter_head(iterator); iter_node(iterator); iter_next(iterator))
    if (iter_node_ptr(iterator) == addr)
      return TRUE;
  return FALSE;
}
