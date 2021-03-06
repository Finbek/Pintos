#include "vm/swap.h"
#include <bitmap.h>
#include "threads/synch.h"

struct lock block_lock;

void init_swap()
{
	swap_block = block_get_role(BLOCK_SWAP);
	swap_table = bitmap_create(block_size(swap_block)/8);
	bitmap_set_all(swap_table, false);
	lock_init(&block_lock);
}

size_t  write_to_block(void* frame)
{
	get_lock(&block_lock);
	size_t index = bitmap_scan_and_flip(swap_table, 0,1,false);
	int i =0;
	for(i; i<8;i++)
	{
		block_write(swap_block, index*8+i,(uint8_t *) frame+i*BLOCK_SECTOR_SIZE);
	}
	give_up_lock(&block_lock);
	return index;
}

void  read_from_block(void* frame, size_t index)
{
	get_lock(&block_lock);
	int i =0;
        for(i; i<8;i++)
        {
                block_read(swap_block, index*8+i, (uint8_t *) frame+i*BLOCK_SECTOR_SIZE);
        }
	bitmap_flip(swap_table, index);
	give_up_lock(&block_lock);
}


