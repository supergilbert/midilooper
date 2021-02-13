/* Copyright 2012-2020 Gilbert Romer */

/* This file is part of midilooper. */

/* midilooper is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* midilooper is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with midilooper.  If not, see <http://www.gnu.org/licenses/>. */


#ifndef __TOOL_H
#define __TOOL_H

#include "tool/compilation_config.h"

typedef enum {
  MSQ_FALSE = 0,
  MSQ_TRUE
}	msq_bool_t;

typedef unsigned int uint_t;
typedef int          int_t;

/* typedef uint_t		tick_t; */

typedef unsigned short	uint16_t;

typedef unsigned char	byte_t;

typedef struct node_s
{
  void          *addr;
  struct node_s *next;
  struct node_s *prev;
} node_t;

typedef struct
{
  node_t *head;
  node_t *tail;
  uint_t len;
} list_t;

#include <stdlib.h>
void *myalloc(size_t size);

node_t *push_to_list(list_t *list, void *addr);
node_t *push_to_list_tail(list_t *list, void *addr);

typedef void (*free_list_func)(void *addr);
void free_list_node(list_t *list, free_list_func func);

typedef void (*list_func)(void *addr, void *args);
void foreach_list_node(list_t *list, list_func func, void *args);

#define NEXT_NODE(node) ((node)->next)
#define PREV_NODE(node) ((node)->prev)

#define LIST_HEAD(list) ((list)->head)
#define LIST_TAIL(list) ((list)->tail)

typedef struct
{
  node_t *node;
  list_t *list;
} list_iterator_t;

void iter_init(list_iterator_t *iterator, list_t *list);
void iter_copy(list_iterator_t *iter_src, list_iterator_t *iter_dst);
msq_bool_t iter_move_to_addr(list_iterator_t *iterator, void *addr);
/* #define iter_init(iterator, list) (iterator)->node = (list)->head; (iterator)->list = (list) */
#define iter_node(iterator) ((iterator)->node)
#define iter_node_ptr(iterator) ((iterator)->node->addr)
#define iter_node_prev_ptr(iterator) ((iterator)->node->prev->addr)

#define iter_next(iterator) ((iterator)->node = (iterator)->node->next)
#define iter_head(iterator) ((iterator)->node = (iterator)->list->head)

node_t *iter_push_before(list_iterator_t *iterator, void *addr);
node_t *iter_push_after(list_iterator_t *iterator, void *addr);
void   iter_node_del(list_iterator_t *iterator, free_list_func func);

#endif
