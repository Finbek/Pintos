#ifndef VM_SWAP_H
#define VM_SWAP_H

#include "devices/block.h"
#include <bitmap.h>
#include "threads/synch.h"
#include "threads/vaddr.h"

 
static struct block* swap_block;
static struct bitmap* swap_table;
void init_swap();
size_t  write_to_block(uint8_t* frame);
void  read_from_block(uint8_t* frame, size_t index);

#endif
