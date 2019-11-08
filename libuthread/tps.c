#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "queue.h"
#include "thread.h"
#include "tps.h"

typedef struct page
{
  int count;
  void *mem_add;
}page, *page_t;

typedef struct tps
{
  pthread_t thread;
  page_t page;
}tps, *tps_t;

queue_t tps_threads;

static int is_tid(void *data, void *arg)
{
  tps_t tps = (tps_t)data;
  pthread_t tid = *(pthread_t*)arg;
  if (tps->thread == tid)
    return 1;
  else
    return 0;
}

static int find_add(void *data, void *arg)
{
  tps_t tps = (tps_t)data;
  if (tps->page->mem_add == arg)
    return 1;
  else
    return 0;
}

static void segv_handler(int sig, siginfo_t *si, void *context)
{
  /*
  * Get the address corresponding to the beginning of the page where the
  * fault occurred
  */
  void *p_fault = (void*)((uintptr_t)si->si_addr & ~(TPS_SIZE - 1));

  /*
  * Iterate through all the TPS areas and find if p_fault matches one of them
  */
  tps_t found = NULL;
  queue_iterate(tps_threads, find_add, p_fault, (void**)&found);

  if (found) /* Printf the following error message */
    fprintf(stderr, "TPS protection error!\n");

  /* In any case, restore the default signal handlers */
  signal(SIGSEGV, SIG_DFL);
  signal(SIGBUS, SIG_DFL);
  
  /* And transmit the signal again in order to cause the program to crash */
  raise(sig);
}

int tps_init(int segv)
{
  if (tps_threads)
    return -1;

  tps_threads = queue_create();
  
  //signal handling
  if(segv) 
  {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = segv_handler;
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGSEGV, &sa, NULL);
  }

  return 0;
}

int tps_create(void)
{
  tps_t check = NULL;
  pthread_t tid = pthread_self();
  queue_iterate(tps_threads, is_tid, (void*)(&tid), (void**)&check);
  if (check)
    return -1;

  tps_t t = (tps_t)malloc(sizeof(tps));
  if (!t)
    return -1;
  t->thread = tid;
  
  page_t p = (page_t)malloc(sizeof(page));
  if (!p)
    return -1;
  t->page = p;
  t->page->count = 1;
  t->page->mem_add = mmap(NULL, TPS_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  queue_enqueue(tps_threads, (void*)t);

  return 0;
}

int tps_destroy(void)
{
  tps_t t = NULL;
  pthread_t tid = pthread_self();
  queue_iterate(tps_threads, is_tid, (void*)&tid, (void**)&t);

  if (!t)
    return -1;

  if (t->page->count == 1)
  {
    free(t->page);
  }
  else
  {
    t->page->count--;
  }

  free(t);
  return 0;
}

int tps_read(size_t offset, size_t length, char *buffer)
{
  //check if thread has a tps
  tps_t tps = NULL;
  pthread_t tid = pthread_self();
  queue_iterate(tps_threads, is_tid, (void*)(&tid), (void**)&tps);
  //if no tps
  if (!tps)
    return -1;

  //if buffer is NULL
  if(buffer == NULL)
    return -1;

  //if offset + memory address + length is too large, outside bounds
  if(length + offset > TPS_SIZE)
    return -1;

  mprotect(tps->page->mem_add, TPS_SIZE, PROT_READ);
  memcpy(buffer, tps->page->mem_add + offset, length);
  mprotect(tps->page->mem_add, TPS_SIZE, PROT_NONE);
  
  return 0;
}

int tps_write(size_t offset, size_t length, char *buffer)
{
  tps_t tps = NULL;
  pthread_t tid = pthread_self();
  queue_iterate(tps_threads, is_tid, (void*)(&tid), (void**)&tps);
  //if current thread doesn't have a tps
  if (!tps)
    return -1;

  //if buffer is NULL
  if(buffer == NULL)
    return -1;

  //if offset + memory address + length is too large, outside bounds
  if(length + offset > TPS_SIZE)
    return -1;

  //more than 1 thread points to this page
  if(tps->page->count > 1)
  {
    page_t new_page = (page_t)malloc(sizeof(page));
    new_page->count = 1;
    new_page->mem_add = mmap(NULL, TPS_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1 , 0);

    mprotect(tps->page->mem_add, TPS_SIZE, PROT_READ);
    memcpy(new_page->mem_add, tps->page->mem_add, TPS_SIZE); // copy reference over
    memcpy(new_page->mem_add + offset, buffer, length); // write buffer into page
    
    mprotect(new_page->mem_add, TPS_SIZE, PROT_NONE);
    mprotect(tps->page->mem_add, TPS_SIZE, PROT_NONE);

    tps->page->count = tps->page->count - 1;
    tps->page = new_page;

  }
  else  // only 1 thread points to this page
  {
    mprotect(tps->page->mem_add, TPS_SIZE, PROT_WRITE);
    memcpy(tps->page->mem_add + offset, buffer, length);
    mprotect(tps->page->mem_add, TPS_SIZE, PROT_NONE);
  }

  return 0;
}

int tps_clone(pthread_t tid)
{
  tps_t current_thread = NULL;
  pthread_t current_tid = pthread_self();
  tps_t given_thread = NULL;
  queue_iterate(tps_threads, is_tid, (void*)(&current_tid), (void**)&current_thread);
  queue_iterate(tps_threads, is_tid, (void*)(&tid), (void**)&given_thread);

  if (!given_thread || current_thread)
    return -1;

  given_thread->page->count = given_thread->page->count + 1;
  current_thread = (tps_t)malloc(sizeof(tps)); 
  current_thread->page = given_thread->page;
  current_thread->thread = current_tid;
  queue_enqueue(tps_threads, (void*)current_thread);

  return 0;
}

