
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
		
		struct frame_table_elem * fte = malloc(sizeof(struct frame_table_elem));
		fte->frame = frame;
		fte->holder = thread_current();
		list_insert_ordered(&frame_table, &fte->elem, list_less, NULL);
		return frame;
	}

bool list_less (const struct list_elem *a,
                             const struct list_elem *b,
                             void *aux UNUSED)
{
	//Change it according the eviction policy time - based
	return list_entry(a, struct thread, elem)->priority<list_entry(b, struct thread, elem)->priority;
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
	return false;
}



