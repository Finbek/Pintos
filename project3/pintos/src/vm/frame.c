
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "vm/frame.h"

//Initialize by this
struct lock frame_lock;
void init_frame_table()
{
	lock_init(&frame_lock);
	list_init(&frame_table);
}

void *falloc(enum palloc_flags flags)
	{
		uint8_t* frame = palloc_get_page(flags);
		if( frame==NULL)
		{
			fevict();
			uint8_t* frame =palloc_get_page(flags);
			ASSERT(frame!=NULL);
		}
		lock_acquire(&frame_lock);
		struct frame_table_elem * f = malloc(sizeof(struct frame_table_elem));
		f->frame = frame;
		f->holder = thread_current();
		list_insert_ordered(&frame_table, &f->elem, list_less, NULL);
		lock_release(&frame_lock);
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
	return list_entry(a, struct frame_table_elem, elem)->page->start_time<list_entry(b, struct frame_table_elem, elem)->page->start_time;
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

bool fevict()
{
	struct frame_table_elem* f = list_entry(list_begin(&frame_table),struct frame_table_elem, elem);
	if(pagedir_is_accessed(f->holder->pagedir, f->page->user_addr))
	{	
		pagedir_set_accessed(f->holder->pagedir, f->page->user_addr, false);
		if( pagedir_is_dirty(f->holder->pagedir, f->page->user_addr))
		{
			file_write_at(f->page->file, f->frame, f->page->page_read_bytes, f->page->offset);
		}
		if(flag_swap_init==false)
		{
			init_swap();
			flag_swap_init = true;
		}
			f->page->status = PAGE_SWAPPED;
			f->page->swap_index = write_to_block(f->frame);	

	}
	pagedir_clear_page(f->holder, f->page->user_addr);
	list_remove(&f->elem);
	palloc_free_page(f->frame);
	free(f);
	return true;
}



