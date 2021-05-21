#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdio.h>
#include "threads/thread.h"
typedef int pid_t;
typedef int mapid_t;
void syscall_init (void);


void halt (void);

void exit (int status);

pid_t exec (const char *cmd_line);

int wait_sys (pid_t pid);

bool create (const char *file, unsigned initial_size);

bool remove (const char *file);

int open (const char *file);

int filesize (int fd);

int read (int fd, void *buffer, unsigned size);

int write (int fd, const void *buffer, unsigned size);

void seek (int fd, unsigned position);

unsigned tell (int fd);

void close (int fd);

void unmap(struct mmap* mmap);

void munmap_all();

void munmap(mapid_t mapping);

mapid_t mmap(int fd, void* addr);

bool validation(void* addr, void* esp);

#endif /* userprog/syscall.h */
