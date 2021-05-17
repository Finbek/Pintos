#include "vm/swap.h"
#include <bitmap.h>

void init_swap()
{
	swap_block = block_get_role(BLOCK_SWAP);
	swap_table = bitmap_create(block_size(swap_block)/8);
	bitmap_set_all(swap_table, false);
}

size_t  write_to_block(uint8_t* frame)
{
	size_t index = bitmap_scan_and_flip(swap_table, 0,1,false);
	ASSERT(index!=BITMAP_ERROR);
	int i =0;
	for(i; i<8;i++)
	{
		block_write(swap_block, index*8+i, frame+(i*BLOCK_SECTOR_SIZE));
	}
	return index;
}

void  read_from_block(uint8_t* frame, size_t index)
{
	ASSERT(bitmap_test(swap_table, index)!=0)
	bitmap_flip(swap_table, index);
	int i =0;
        for(i; i<8;i++)
        {
                block_write(swap_block, index*8+i, frame+(i*BLOCK_SECTOR_SIZE));
        }
}


