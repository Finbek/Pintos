#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "threads/vaddr.h"
#include "userprog/pagedir.h"
static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  //printf ("system call!\n");
  if(is_user_vaddr(f->esp) && pagedir_get_page(thread_current()->pagedir,(f->esp)))
  {
  	int code = *(int*)f->esp;
	
	if(code==SYS_HALT)
		halt();
	if(code == SYS_EXIT)
	{
		int status = *((int*)f->esp+1);
		exit(status);
	}
	if(code ==SYS_EXEC)
	{
		const char* cmd_line = (char*)(*((int*)f->esp+1));
		f->eax=exec(cmd_line);//CHECK THIS PID CHILD
		 
	} 
	if(code == SYS_WAIT)
	{
		pid_t pid =(pid*) (*((int*)f->esp+1));
		f->eax = wait(pid);
	}
	if(code == SYS_CREATE){
		const char* file = (char*)(*((int*)f->esp+1));
		unsigned initial_size =*((unsigned*)f->esp+2);
		f->eax = create(file, initial_size);
	}
	if(code == SYS_REMOVE)
	{
		const char* file = (char*)(*((int*)f->esp+1));
		f->eax = remove(file);
	}
	if(code == SYS_OPEN)
	{	
		const char* file = (char*)(*((int*)f->esp+1));
		f->eax = open(file);
	}
	if(code == SYS_FILESIZE)
	{
		int fd = *((int*)f->esp+1);
		f->eax = filesize(fd);
	}
	if(code == SYS_READ)
	{
		
		int fd = *((int*)f->esp+1);
		void* buffer = (void*)(*((int*)f->esp+2));
		unsigned size = *((unsigned*)f->esp+3);
		f->eax = read(fd, buffer, size);
	}
	if(code == SYS_WRITE)
	{
		int fd = *((int*)f->esp+1);
		void* buffer = (void*)(*((int*)f->esp+2));
		unsigned size = *((unsigned*)f->esp+3);
		f->eax = write(fd, buffer, size);
	}		
	if(code == SYS_SEEK)
	{
		
		int fd = *((int*)f->esp+1);
		unsigned position = *((unsigned*)f->esp+3);
		seek = (fd, position);
	}
	if(code == SYS_TELL)
	{
		int fd = *((int*)f->esp+1);
		f->eax = tell(fd);
	}
	if(code == SYS_CLOSE)
 	{
		int fd = *((int*)f->esp+1);
		close(fd);	
	}		
  }
  else
  {
  	printf("not valid address\n");
  	exit(-1);
  }
}
