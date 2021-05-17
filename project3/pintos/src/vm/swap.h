#ifndef VM_SWAP_H
#define VM_SWAP_H

#include "devices/block.h"
#include <bitmap.h>
#include "threads/synch.h"
#include "threads/vaddr.h"

 
static struct block* swap_block;
static struct bitmap* swap_table;

void init_swap();

#endif
