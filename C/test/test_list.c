#include "./tool/tool.h"
#include <strings.h>
#include <stdio.h>

int main()
{
  list_t list;
  list_iterator_t iter;

  bzero(&list, sizeof (list_t));
  bzero(&iter, sizeof (list_iterator_t));

  push_to_list_tail(&list, (void *) 1);
  push_to_list_tail(&list, (void *) 2);
  push_to_list_tail(&list, (void *) 3);
  push_to_list_tail(&list, (void *) 4);
  push_to_list_tail(&list, (void *) 5);
  push_to_list_tail(&list, (void *) 6);

  for (iter_init(&iter, &list);
       iter_node(&iter);
       iter_next(&iter)) {
    iter_push_before(&iter, NULL);
  }

  push_to_list(&list, (void *) 42);
  push_to_list_tail(&list, (void *) 255);


  for (iter_init(&iter, &list);
       iter_node(&iter);
       iter_next(&iter)) {
    printf(" addr = %p", iter_node_ptr(&iter));
  }
  return 0;
}
