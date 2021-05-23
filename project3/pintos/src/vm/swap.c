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

size_t  write_to_block(uint8_t* frame)
{
	lock_acquire(&block_lock);
	size_t index = bitmap_scan_and_flip(swap_table, 0,1,false);
	int i =0;
	for(i; i<8;i++)
	{
		block_write(swap_block, index*8+i, frame+(i*BLOCK_SECTOR_SIZE));
	}
	lock_release(&block_lock);
	return index;
}

void  read_from_block(uint8_t* frame, size_t index)
{
	lock_acquire(&block_lock);
	int i =0;
        for(i; i<8;i++)
        {
                block_read(swap_block, index*8+i, frame+(i*BLOCK_SECTOR_SIZE));
        }
	bitmap_flip(swap_table, index);
	lock_release(&block_lock);
}


