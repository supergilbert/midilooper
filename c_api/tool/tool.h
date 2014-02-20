#ifndef __TOOL_H
#define __TOOL_H

#include "tool/compilation_config.h"

typedef enum {
  FALSE = 0,
  TRUE
}	bool_t;

typedef unsigned int	uint32_t;
typedef uint32_t	uint_t;

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

#define COPY_LIST_NODE(src, dest) (dest)->head = (src)->head; (dest)->tail = (src)->tail

typedef struct
{
  node_t *node;
  list_t *list;
} list_iterator_t;

void iter_init(list_iterator_t *iterator, list_t *list);
void iter_copy(list_iterator_t *iter_src, list_iterator_t *iter_dst);
bool_t iter_move_to_addr(list_iterator_t *iterator, void *addr);
/* #define iter_init(iterator, list) (iterator)->node = (list)->head; (iterator)->list = (list) */
#define iter_node(iterator) ((iterator)->node)
#define iter_node_ptr(iterator) ((iterator)->node->addr)
#define iter_node_prev_ptr(iterator) ((iterator)->node->prev->addr)

#define iter_next(iterator) ((iterator)->node = (iterator)->node->next)
#define iter_head(iterator) ((iterator)->node = (iterator)->list->head)

node_t *iter_push_before(list_iterator_t *iterator, void *addr);
node_t *iter_push_after(list_iterator_t *iterator, void *addr);
void iter_node_del(list_iterator_t *iterator, free_list_func func);

#endif
