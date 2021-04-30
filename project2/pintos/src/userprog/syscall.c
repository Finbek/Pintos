#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "filesys/filesys.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "threads/malloc.h"
#include <list.h>
static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}
struct file_fd* find_file_fd(int fd_number);
static void
syscall_handler (struct intr_frame *f) 
{
  //printf ("system call!\n");
  if(is_user_vaddr(f->esp) && pagedir_get_page(thread_current()->pagedir,(f->esp)!=NULL))
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
		pid_t pid =(pid_t*) (*((int*)f->esp+1));
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
		seek(fd, position);
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
  	//printf("not valid address\n");
  	exit(-1);
  }
}

void 
halt (void)
{
  shutdown_power_off();
}

void exit (int status)
{

}

pid_t
exec (const char *cmd_line)
{
return -1;
}

int wait (pid_t pid)
{
return -1;
}

bool create (const char *file, unsigned initial_size)
{
//Synchronization LOOOOOOK UP LATER
  bool success = false; 
  if(is_user_vaddr(file) && pagedir_get_page(thread_current()->pagedir,(file)!=NULL))
	success = filesys_create(file, initial_size);
  return success;	
}

bool remove (const char *file)
{
    bool success = false;
  if(is_user_vaddr(file) && pagedir_get_page(thread_current()->pagedir,(file)!=NULL))
	success = filesys_remove(file);
  return success;
}

int open (const char *file)
{
  int success = -1;
  static int fd_number = 2;
  if(is_user_vaddr(file) && pagedir_get_page(thread_current()->pagedir,(file)!=NULL))
	{
		struct file* open_file = filesys_open(file);
		struct file_fd* fd = (struct file_fd*) malloc(sizeof(struct file_fd));
		fd ->fd_numb = fd_number;
		fd->file = open_file;
		fd+=1;
		list_push_front(&thread_current()->list_fd, &fd->elem);
		success =fd_number;		
	}
  return success;
	
}

struct file_fd* find_file_fd(int fd_number)
	{	 struct list_elem* first = list_begin(&thread_current()->list_fd);
	 	 struct list_elem* last = list_end(&thread_current()->list_fd);
		struct file_fd* a; 
		while (first!=last)
		 {	a = list_entry(first, struct file_fd, elem);
			if(a->fd_numb ==fd_number)
			{
				return a;
	       		 }
	 
		}
		a = list_entry(first, struct file_fd, elem);
                        if(a->fd_numb ==fd_number)
                        {
                                return a;
                         }

}


int filesize (int fd)
{
  int success = -1;
  struct file_fd* a = find_file_fd(fd);
  if(a!=NULL)
	success = file_length(a->file);
	
  return success;
}

int read (int fd, void *buffer, unsigned size)
{
  int success = -1;
  if(is_user_vaddr(buffer) && pagedir_get_page(thread_current()->pagedir,(buffer)!=NULL))
        {	if(fd ==0)
		{	int i =0;
			while(i<size)
			{
				*((char *)buffer++) = input_getc();
				i+=1;
			}
		
		return i;
		}
		else{
		
                	struct file_fd* a = find_file_fd(fd);
                	if(a!=NULL)
                        	success = file_read(a->file, buffer, size);
        }
        }
  return success;
}

int write (int fd, const void *buffer, unsigned size)
{	
int success =0;
	if(fd==0){
		putbuf(buffer, size);
		return size;
	}
	struct file_fd* a = find_file_fd(fd);
              if(a!=NULL)
                        success = file_write(a->file, buffer, size);
	return success;
}

void seek (int fd, unsigned position)
{
	 struct file_fd* a = find_file_fd(fd);
      	 struct file* f = a->file; 
			file_seek(f, position);

}

unsigned tell (int fd)
{
	struct file_fd* a = find_file_fd(fd);
                if(a!=NULL)
                	return file_tell(a->file);
	return 0;
}

void close (int fd)
{
	struct file_fd* a = find_file_fd(fd);
	file_close(a->file);
	list_remove(&a->elem);  
}
