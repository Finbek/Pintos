
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "vm/frame.h"

//Initialize by this

void init_frame_table()
{
	list_init(&frame_table);
}

void *falloc(enum palloc_flags flags)
	{
		uint8_t* frame = palloc_get_page(flags);
		if( frame==NULL)
		{
			PANIC("NO SPACE");
			//evict
			//reallocate
		}
		
		struct frame_table_elem * f = malloc(sizeof(struct frame_table_elem));
		f->frame = frame;
		f->holder = thread_current();
		list_insert_ordered(&frame_table, &f->elem, list_less, NULL);
		return frame;
	}

void update_order(uint8_t *frame)
	{
		struct frame_table_elem * f = find_frame(frame);
		list_remove(&f->elem);
		list_insert_ordered(&frame_table, &f->elem, list_less, NULL);
	}
bool list_less (const struct list_elem *a,
                             const struct list_elem *b,
                             void *aux UNUSED)
{
	//Change it according the eviction policy time - based
	printf("A elem: %s\n",list_entry(a, struct frame_table_elem, elem)->page->time);
	printf("B elem: %s\n",list_entry(b, struct frame_table_elem, elem)->page->time);
	printf("A is less than B");
	printf(list_entry(a, struct frame_table_elem, elem)->page->time<list_entry(b, struct frame_table_elem, elem)->page->time);
	return list_entry(a, struct frame_table_elem, elem)->page->time<list_entry(b, struct frame_table_elem, elem)->page->time;
}



void f_free(void* frame)
{
	struct frame_table_elem * f = find_frame(frame);
	if(f !=NULL)
	{
		list_remove(&f->elem);
		free(f);
	}
	palloc_free_page(frame);	
	
}

struct frame_table_elem*  find_frame(void * frame)
{	
	struct frame_table_elem* f;
	struct list_elem *e;
        for (e = list_begin(&frame_table); e != list_end (&frame_table); e = list_next(e))
        {
                f = list_entry(e, struct frame_table_elem, elem);
                if (f->frame ==frame)
                        return f;
        }
        return NULL;
}

bool fevict(void *frame)
{
	struct frame_table_elen* f = list_entry(list_begin(&frame_table),struct frame_table_elem, elem);
	if(pagedir_is_dirty(f->holder->pagedir, f->page->addr))
	{
		f->page->status = SWAP;
		f->page->swap_index = write_to_block(f->frame);	
		//write to the file

	}
	list_remove(&f->elem);
	palloc_free_page(f->frame);
	free(f);
	return true;
}



