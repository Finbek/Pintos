#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "hash.h"
enum page_status{
	PAGE_SWAPPED,
	PAGE_LOADED,
	PAGE_ALLOCATED,
	PAGE_DEL
};

struct sup_page{
	unsigned start_time;
	uint32_t* user_addr;
	struct thread* holder;
	struct hash_elem elem;
	enum page_status status;
	int swap_index;
	uint8_t *frame;
	//File information
	size_t page_read_bytes;
	size_t page_zero_bytes;
	bool writtable;
	off_t offset;
	struct file* file;
		
};

struct sup_page* sp_alloc(struct file *file, off_t ofs, uint8_t *upage,uint32_t read_bytes, uint32_t zero_bytes, bool writable);
void spt_init();
bool page_fault_handler(void* fault_addr, uint32_t esp);
unsigned hash_func(const struct hash_elem*, void *aux UNUSED);
bool hash_less(const struct hash_elem*, const struct hash_elem*, void *aux UNUSED);
#endif
