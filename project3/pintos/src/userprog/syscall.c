#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"

#include "filesys/filesys.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "threads/malloc.h"
#include <list.h>
#include "threads/synch.h"
#include "lib/kernel/list.h"
#include "vm/page.h"
#include "vm/frame.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  lock_init(&critical_section);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}
struct file_fd* find_file_fd(int fd_number);

bool validation(void * addr, void*esp);

static void
syscall_handler (struct intr_frame *f) 
{
  if(validation((int*)f->esp, f->esp))
  {
  	int code = *(int*)f->esp;
	if(code==SYS_HALT)
		halt();
	if(code == SYS_EXIT)
	{	
		if(!validation(((int*)f->esp+1),f->esp))
			exit(-1);
		else
		{
		int status = *((int*)f->esp+1);
		exit(status);
		}
	}
	if(code ==SYS_EXEC)
	{
		const char* cmd_line = (char*)(*((int*)f->esp+1));
		if(!validation(cmd_line,f->esp))
                        exit(-1);
                else
                {
		f->eax=exec(cmd_line);//CHECK THIS PID CHILD
		 }
	} 
	if(code == SYS_WAIT)
	{
		if(!validation(((int*)f->esp+1),f->esp))
                        exit(-1);
                else
                {
		pid_t pid =(pid_t*) (*((int*)f->esp+1));
		f->eax = wait_sys(pid);
		}
	}
	if(code == SYS_CREATE){
                
		const char* file = (char*)(*((int*)f->esp+4));
		unsigned initial_size =*((unsigned*)f->esp+5);
		if (validation(file,f->esp))
			f->eax = create(file, initial_size);
		else
			exit(-1);
	}
	if(code == SYS_REMOVE)
	{
		const char* file = (char*)(*((int*)f->esp+1));
		if(validation(file,f->esp))
			f->eax = remove(file);
		else
			exit(-1);
	}
	if(code == SYS_OPEN)
	{
		const char* file = (char*)(*((int*)f->esp+1));
		if(validation(file,f->esp))
			f->eax = open(file);
		else
			exit(-1);
	}
	if(code == SYS_FILESIZE)
	{	
		int fd = *((int*)f->esp+1);
		f->eax = filesize(fd);
	}
	if(code == SYS_READ)
	{
		
		int fd = *((int*)f->esp+5);
		char* buffer = (char*)(*((int*)f->esp+6));
		unsigned size = *((unsigned*)f->esp+7);
		unsigned i =0;
                while(i<=size)
                        {
                                if(validation((void*)buffer+i, f->esp)==false)
                                        exit(-1);

                        i++;
                        }
		f->eax = read(fd, (void*)buffer, size);
	}
	if(code == SYS_WRITE)
	{
                	int fd = *((int*)f->esp+5);
                	char* buffer = (char*)(*((int*)f->esp+6));
                	unsigned size = *((unsigned*)f->esp+7);
			unsigned i =0;
			while(i<=size)
			{
				if(validation((void*)buffer+i, f->esp)==false)
					exit(-1);
		
			i++;
			}
			f->eax = write(fd,(void*) buffer, size);
	}		
	if(code == SYS_SEEK)
	{
		int fd = *((int*)f->esp+4);
		unsigned position = *((unsigned*)f->esp+5);
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
	if(code ==SYS_MMAP)
	{
		int fd = *((int*)f->esp+4);
		void* addr = (void*)(*((int*)f->esp+5));
		if (validation(addr,f->esp))
			f->eax = mmap(fd, addr);
		else
			exit(-1);
	}
	if (code ==SYS_MUNMAP)	
	{
		mapid_t mapping = *((int*)f->esp+1);
		munmap(mapping); 
	}
  }
  else
  {
  	exit(-1);
  } 
}

bool validation(void* addr, void* esp)

{
	if (addr!=NULL && is_user_vaddr((void*)addr)){
		if (page_fault_handler((void*) addr, esp)==true)
			return true;
	}
	return false;
}


void 
halt (void)
{
  shutdown_power_off();
}

void close_fds()
	{
                struct list_elem* first = list_begin(&thread_current()->list_fd);
                struct list_elem* last = list_end(&thread_current()->list_fd);
                struct file_fd* a;
                while (first!=last)
                 {      a = list_entry(first, struct file_fd, elem);
                        first=list_next(first);
			close(a->fd_numb);
                }
	}

void remove_status_keeper()
{
	struct thread *t = thread_current();
	if(list_size(&t->status_list)!=0)
        {
             struct list_elem *e;
             struct child* ch;
             for(e = list_begin(&t->status_list); e != list_end(&t->status_list); e = list_next(e))
             {
                  ch = list_entry(e, struct child, elem);
                  list_remove(&ch->elem);
                  free(ch);
             }
       	}
}



void exit (int status)
{
   close_fds();
   munmap_all();
   spt_free(&thread_current()->spt);
   if(lock_held_by_current_thread(&critical_section))
	lock_release(&critical_section);
   struct thread *t = thread_current();
   if(t->executable!=NULL)
	{
		file_allow_write(t->executable);
		close(t->executable);
	}
	remove_status_keeper();
   if (t->is_child)
   {
     struct thread *parent = find_thread(t->parent);
     if (parent != NULL)
     {
	struct child * status_child = (struct child*) malloc(sizeof(struct child));
	status_child->status = status;
	status_child->tid = t->tid;
	list_push_front(&parent->status_list, &status_child->elem);
   	printf ("%s: exit(%d)\n", t->name, status);
	list_remove(&t->child_elem);
   	sema_up(&parent->parent_sleep);
	thread_exit();
        return;
     }
   }
   printf ("%s: exit(%d)\n", t->name, status);
   thread_exit();
}

pid_t
exec (const char *cmd_line)
{
  if (!lock_held_by_current_thread(&critical_section))
      while(!lock_try_acquire(&critical_section))
              thread_yield();
  pid_t pid = process_execute(cmd_line);
  if (lock_held_by_current_thread(&critical_section))
      lock_release(&critical_section);
  
  if (pid == TID_ERROR)
  {
     return -1;
  }
  return pid;
}

int wait_sys (pid_t pid)
{
  return process_wait(pid);
}

bool create (const char *file, unsigned initial_size)
{
  bool success = false; 
	if (!lock_held_by_current_thread(&critical_section))
		while(!lock_try_acquire(&critical_section))
        	        thread_yield();
	if (file == NULL)
	{
		exit(-1);
	} else 
	{
		success = filesys_create(file, initial_size);
	}
	if (lock_held_by_current_thread(&critical_section))
		lock_release(&critical_section);
  return success;	
}

bool remove (const char *file)
{
    bool success = false;
	if (!lock_held_by_current_thread(&critical_section))
		while(!lock_try_acquire(&critical_section))
                	thread_yield();
	success = filesys_remove(file);
	if (lock_held_by_current_thread(&critical_section))
		lock_release(&critical_section);
  return success;
}

int open (const char *file)
{
  
  int success = -1;
  if (file == NULL)
     return success;
  static int fd_number = 2;
		if (!lock_held_by_current_thread(&critical_section))
			while(!lock_try_acquire(&critical_section))
        	        	thread_yield();
		struct file* open_file = filesys_open(file);
		if (lock_held_by_current_thread(&critical_section))
                                lock_release(&critical_section);
		if(open_file==NULL)
		{
			return -1;
		}
		struct file_fd* fd =  malloc(sizeof(struct file_fd));
		fd ->fd_numb = fd_number;
		fd->file = open_file;
		fd_number+=1;
		struct list* add_here = &thread_current()->list_fd;
		list_push_front(add_here, &fd->elem);
		success =fd->fd_numb;
		
	if (lock_held_by_current_thread(&critical_section))
		lock_release(&critical_section);
  return success;
	
}

struct file_fd* find_file_fd(int fd_number)
	{	 
		struct list_elem* first = list_begin(&thread_current()->list_fd);
	 	 struct list_elem* last = list_end(&thread_current()->list_fd);
		struct file_fd* a; 
		while (first!=last)
		{	a = list_entry(first, struct file_fd, elem);
			if(a->fd_numb ==fd_number)
			{
				return a;
	       		 }
			first = list_next(first);
	 
		}
		return NULL;

}


int filesize (int fd)
{
  int success = -1;
  struct file_fd* a = find_file_fd(fd);
  if(a!=NULL)
	if (!lock_held_by_current_thread(&critical_section))
		while (!lock_try_acquire(&critical_section))
	  		thread_yield();
	success = file_length(a->file);
	if (lock_held_by_current_thread(&critical_section))
		lock_release(&critical_section);
	
  return success;
}

int read (int fd, void *buffer, unsigned size)
{
  int success = -1;
	if (!lock_held_by_current_thread(&critical_section))
		while(!lock_try_acquire(&critical_section))
			thread_yield();
 	if(fd ==0)
		{	int i =0;
			while(i<size)
			{
				*((char *)buffer++) = input_getc();
				i+=1;
			}
		
		if (lock_held_by_current_thread(&critical_section))
			lock_release(&critical_section);
		return i;
		}
		else{
		
                	struct file_fd* a = find_file_fd(fd);
                	if(a!=NULL)
                        	success = file_read(a->file, buffer, size);
       		 }
        
	if (lock_held_by_current_thread(&critical_section))
		lock_release(&critical_section);
  return success;
}

int write (int fd, const void *buffer, unsigned size)
{	
	int success =0;
	if (!lock_held_by_current_thread(&critical_section))
		while(!lock_try_acquire(&critical_section))
			thread_yield();
	if(fd==1){
		putbuf(buffer, size);
		if (lock_held_by_current_thread(&critical_section))
			lock_release(&critical_section);
		return size;
	}
	struct file_fd* a = find_file_fd(fd);
              if(a!=NULL)
                        success = file_write(a->file, buffer, size);
	if (lock_held_by_current_thread(&critical_section))
		lock_release(&critical_section);
	return success;
}

void seek (int fd, unsigned position)
{
	 struct file_fd* a = find_file_fd(fd);
      	 struct file* f = a->file; 
	if (!lock_held_by_current_thread(&critical_section))
		while(!lock_try_acquire(&critical_section))
			thread_yield();	
		file_seek(f, position);
	if (lock_held_by_current_thread(&critical_section))
		lock_release(&critical_section);

}

unsigned tell (int fd)
{
	unsigned success = -1;
	struct file_fd* a = find_file_fd(fd);
                if(a!=NULL)
{		if (!lock_held_by_current_thread(&critical_section))
			while(!lock_try_acquire(&critical_section))
                		thread_yield();
                 success = file_tell(a->file);
		if (lock_held_by_current_thread(&critical_section))
			lock_release(&critical_section);}
	return success;
}

void close (int fd)
{
	struct file_fd* a = find_file_fd(fd);
	if(a!=NULL)
	{
	if (!lock_held_by_current_thread(&critical_section))
		while(!lock_try_acquire(&critical_section))
                	thread_yield();
	file_close(a->file);
	if (lock_held_by_current_thread(&critical_section))
		lock_release(&critical_section);
	list_remove(&a->elem);
	free(a);
	} 
}

mapid_t mmap(int fd, void* addr)
{
	if(fd==1 || fd==0)
		return -1;
	struct file_fd* file_fd_opened = find_file_fd(fd);
	if(file_fd_opened==NULL || file_length(file_fd_opened->file)==0 || addr==NULL || pg_ofs(addr)!=0)		return -1; 
	struct file* file = file_reopen(file_fd_opened->file);
	if(file==NULL)
		return -1;
	//main function
	static mapid_t id = 0;
	off_t ofs = 0;
	uint32_t read_bytes = file_length(file);
	while(read_bytes>0)
	{
     	 	size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
      		size_t page_zero_bytes = PGSIZE - page_read_bytes;
		struct sup_page* sp =  sp_alloc(file, ofs, addr, page_read_bytes, page_zero_bytes, true);
		//Creating mmap struct
		struct mmap* m = malloc(sizeof(struct mmap));
		m->page = sp;
		m->mapid = id;
		m->fd = fd;
		list_push_front(&thread_current()->mm_list, &m->elem);
			
		read_bytes -=page_read_bytes;
		addr += PGSIZE;
		ofs += page_read_bytes;
	}
	id++;
	return id-1;
	
}

void munmap(mapid_t mapping)
{
	struct list_elem* first = list_begin(&thread_current()->mm_list);
        struct list_elem* last = list_end(&thread_current()->mm_list);
        struct mmap* a;
        while (first!=last)
        {      a = list_entry(first, struct mmap, elem);
		if(a->mapid==mapping)
		{
                      unmap(a);
		}
                first=list_next(first);
        }
}
void munmap_all()
{
	struct list_elem* first = list_begin(&thread_current()->mm_list);
        struct list_elem* last = list_end(&thread_current()->mm_list);
        struct mmap* a;
        while (first!=last)
        {      a = list_entry(first, struct mmap, elem);
               unmap(a);
               first=list_next(first);
        }

}


void unmap(struct mmap* mmap)
{
	mmap->page->status = PAGE_DEL;
	if(pagedir_is_dirty(thread_current()->pagedir, mmap->page->user_addr))
	{	file_write_at(mmap->page->file, mmap->page->user_addr, mmap->page->page_read_bytes, mmap->page->offset);
	}
	hash_delete(&thread_current()->spt, &mmap->page->elem);
	list_remove(&mmap->elem);
	pagedir_clear_page(thread_current()->pagedir, mmap->page->user_addr);
	file_close(mmap->page->file);
	free(mmap);
}
