#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "threads/synch.h"
#include "threads/thread.h"
#include <list.h>
#include <stdint.h>

struct list frame_table;
 
struct frame_table_elem {
	uint32_t * frame;
	struct thread* holder;
	struct list_elem elem;
	//add other members
};

void *falloc(enum palloc_flags flags);
bool list_less (const struct list_elem *a,const struct list_elem *b,void *aux UNUSED);
void f_free(void* frame);
void init_frame_table();
struct frame_table_elem*  find_frame(void * frame);
bool fevict(void *frame);







#endif
