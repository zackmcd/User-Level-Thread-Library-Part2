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

int tps_init(int segv)
{
  if (tps_threads)
    return -1;

  tps_threads = queue_create();
  
  //signal handling
  /*if(segv) 
  {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = segv_handler;
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGSEGV, &sa, NULL);
  }*/

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
  //t->page->mem_add = //mmap;
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

  if (t->page->count <= 1)
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

  void* add_oset = tps->page->mem_add + offset;
  void* out_bounds = tps->page->mem_add + TPS_SIZE;
  void* mem_read = add_oset + length;

  //if offset + memory address + length is too large, outside bounds
  if(mem_read > out_bounds)
    return -1;

  memcpy(buffer, add_oset, length);

  if(buffer == NULL)
    return -1;
  
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

  void* add_oset = tps->page->mem_add + offset;
  void* out_bounds = tps->page->mem_add + TPS_SIZE;
  void* mem_read = add_oset + length;

  if(mem_read > out_bounds)
    return -1;

  //if(tps->page->count > 1)
    //copy-on-write first
  memset(add_oset, *buffer, length);

  //if(buffer == NULL)
    return -1;

  return 0;
}

int tps_clone(pthread_t tid)
{
  return 0;
}

