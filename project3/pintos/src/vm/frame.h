#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "threads/synch.h"

struct frame_table_elem {
	uint32_t * frame;
	struct thread* holder;
	//add other members
};








#endif
