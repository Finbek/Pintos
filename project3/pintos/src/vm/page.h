#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "hash.h"
enum page_status{
	PAGE_SWAPPED,
	PAGE_LOADED,
	PAGE_RUNNING
};

struct sup_page{
	unsigned start_time;
	uint32_t* user_addr;
	struct thread* holder;
	struct hash_elem elem;
	enum page_status status;
	int swap_index;
}

void spt_init();
void sp_alloc(uint32_t*);
unsigned hash_func(const structhash_elem*, void *aux UNUSED);
bool hash_less(const struct hash_elem*, const struct hash_elem*, void *aux UNUSED);

#endif
