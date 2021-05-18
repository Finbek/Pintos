#include "page.h"
#include <hash.h>
#include "threads/malloc.h"
#include "threads/thread.h"

void 
spt_init (){
	hash_init(&spt, hash_func, hash_less, NULL);
}

struct sup_page* sp_alloc(uint32_t* kpage){
	falloc(kpage);
	struct sup_page* sp = malloc(sizeof(struct sup_page));
        sp->start_time = 0;//change later
	sp->user_addr = kpage;
	sp->holder = thread_current();
        sp->status = RUNNING;
	sp->swap_index = -1;        
	hash_insert(&spt, &sp->elem);
        return sp;
}

bool hash_less(const struct hash_elem* a, const struct hash_elem* b, void* aux UNUSED)
{
	const struct sup_page* page_a = hash_entry(a, struct sup_page, hash_elem);
	const struct sup_page* page_b = hash_entry(b, struct sup_page, hash_elem);
	return (a->user_addr < b_user_addr);
}

unsigned hash_func(const struct hash_elem* elem, void* aux UNUSED){
	const struct sup_page* p = hash_entry(elem, struct sup_page, hash_elem);
	return hash_bytes (&p->user_addr, sizeof p->user_addr);
}
