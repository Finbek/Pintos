#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "threads/synch.h"
#include "threads/thread.h"
#include <list.h>
#include <stdint.h>
#include "vm/page.h"
struct list frame_table;
 
struct frame_table_elem {
	uint32_t * frame;
	struct sup_page* page;
	struct list_elem elem;
	struct thread* holder;
	//add other members
};

void *falloc(enum palloc_flags flags);
bool list_less (const struct list_elem *a,const struct list_elem *b,void *aux UNUSED);
void f_free(void* frame);
void init_frame_table();
struct frame_table_elem*  find_frame(void * frame);
bool fevict(void *frame);







#endif
