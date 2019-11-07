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
  return 0;
}

int tps_create(void)
{
  if (!tps_threads)
    tps_init();

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
  t->page->mem_add = //mmap;
  queue_enqueue(tps_threads, (void*)t);

  return 0;
}

int tps_destroy(void)
{
  return 0;
}

int tps_read(size_t offset, size_t length, char *buffer)
{
  
  return 0;
}

int tps_write(size_t offset, size_t length, char *buffer)
{
  return 0;
}

int tps_clone(pthread_t tid)
{
  return 0;
}

