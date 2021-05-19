#include "vm/page.h"
#include <hash.h>
#include "threads/malloc.h"
#include "threads/thread.h"
#include "devices/timer.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#define STACK_CHECK (PHYS_BASE - 8*(1024*1024))

static bool hash_bool;

bool hash_less(const struct hash_elem* a, const struct hash_elem* b, void* aux UNUSED)
{
        const struct sup_page* page_a = hash_entry(a, struct sup_page, elem);
        const struct sup_page* page_b = hash_entry(b, struct sup_page, elem);
        return (page_a->user_addr < page_b->user_addr);
}

unsigned hash_func(const struct hash_elem* e, void* aux UNUSED){
        const struct sup_page* p = hash_entry(e, struct sup_page, elem);
       return hash_bytes (&p->user_addr, sizeof p->user_addr);
}

void 
spt_init (){
	hash_bool = hash_init(&thread_current()->spt, hash_func, hash_less, NULL);
}

struct sup_page* sp_alloc(struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable){
	printf("SP_ALLOC");
	struct sup_page* sp = malloc(sizeof(struct sup_page));
        sp->start_time = timer_ticks();//change later
	sp->user_addr = upage;
	sp->holder = thread_current();
        sp->status = PAGE_ALLOCATED;
	sp->swap_index = NULL;
	//File Info
	sp->file = file;
	sp->page_read_bytes=read_bytes;        
	sp->page_zero_bytes=zero_bytes;
	sp->writtable = writable;
	sp->offset = ofs;
	printf("%d\n",hash_bool);
	//Inserting to hash        
	hash_insert(&(thread_current()->spt), &sp->elem);
	printf("SP_ALLOC: END");
	
        return sp;
}

bool page_fault_handler(void* fault_addr, uint32_t esp)
{
	struct sup_page page;
	page.user_addr = (void*)pg_round_down(fault_addr);
	struct hash_elem * h = hash_find(&thread_current()->spt, &page.elem);
	if(h) 
		return page_status_handler(hash_entry(h, struct sup_page, elem)); 
	else if(page.user_addr > STACK_CHECK && (uint32_t*)fault_addr>=(esp-32))
		return stack_growth(pg_round_down(fault_addr));
	return false;
}
	
bool page_status_handler(struct sup_page* page)
{	if(page->status ==PAGE_DEL || page->status ==PAGE_LOADED)
		return false;
	uint8_t * frame;
	if(flag_frame_init==false)
	{	
		init_frame_table();
		flag_frame_init=true;
	}
	if(page->page_read_bytes ==0)
		frame = falloc(PAL_USER|PAL_ZERO);
	else
		frame=falloc(PAL_USER);

	if(page->status==PAGE_SWAPPED)
		read_from_block(frame, page->swap_index);
	if(page->page_read_bytes!=0 &&  page->status == PAGE_ALLOCATED)
	{
		file_read_at(page->file, frame,page->page_read_bytes, page->offset);
		memset(frame+page->page_read_bytes, 0 , page->page_zero_bytes);
	}
	bool inst = install_page(page->user_addr, frame, page->writtable);
	page->status = PAGE_LOADED;
	return true;
} 
		
bool stack_growth(void* user_addr)
{
	printf("STACK GROW\n");
	struct sup_page* sp = malloc(sizeof(struct sup_page));
	sp->writtable = true;
	sp->user_addr = user_addr;
	sp->status = PAGE_ALLOCATED;
	uint8_t * frame = falloc(PAL_USER|PAL_ZERO);
	install_page(sp->user_addr, frame, true);
	hash_insert(&thread_current()->spt, &sp->elem);
	return true;
	
}		

		

