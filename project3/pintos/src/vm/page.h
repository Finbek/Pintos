#ifndef VM_PAGE_H
#define VM_PAGE_H
#include "hash.h"

#include "filesys/filesys.h"

enum page_status{
	PAGE_SWAPPED,
	PAGE_LOADED,
	PAGE_ALLOCATED,
	PAGE_DEL
};

struct sup_page{
	unsigned start_time;
	void* user_addr;
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
bool stack_growth(void* fault_addr);
bool page_status_handler(struct sup_page* page);
void spt_init();
bool page_fault_handler(void* fault_addr, uint32_t esp);
unsigned hash_func(const struct hash_elem*, void *aux);
bool hash_less(const struct hash_elem*, const struct hash_elem*, void *aux);
#endif
