#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "hash.h"
struct page{
	
	uint32_t* user_addr;
	struct thread* holder;
	struct hash_elem hash_elem;
}









#endif
