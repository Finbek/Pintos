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
	//File information
	size_t page_read_bytes;
	size_t page_zero_bytes;
	bool writtable;
	off_t offset;
	struct file* file;
		
};
static bool flag_frame_init = false;
struct sup_page* sp_alloc(struct file *file, off_t ofs, uint8_t *upage,uint32_t read_bytes, uint32_t zero_bytes, bool writable);
bool stack_growth(void* fault_addr);
bool page_status_handler(struct sup_page* page);
void spt_init();
spt_free(struct hash* h);
bool page_fault_handler(void* fault_addr, uint32_t esp);
#endif
