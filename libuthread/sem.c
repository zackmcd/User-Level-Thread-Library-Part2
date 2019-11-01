#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "thread.h"

struct semaphore {
  int count;
  queue_t blocked_threads;
};

sem_t sem_create(size_t count)
{
  sem_t sem = (sem_t)malloc(sizeof(sem_t));

  if (!sem)
    return NULL;

  sem->count = count;
  sem->blocked_threads = queue_create();
  
  if (!sem->blocked_threads)
    return NULL;
  
  return sem;
}

int sem_destroy(sem_t sem)
{
  if (!sem)
    return -1;
  
  free(sem);
  return 0;
}

int sem_down(sem_t sem)
{
  if (!sem)
    return -1;

  enter_critical_section();

  if (sem->count > 0) // there is a resource to be used
  {
    sem->count--;
    exit_critical_section();
    return 0;
  }
  else // if there are no more resources available and we must block the thread
  {
    pthread_t tid = pthread_self();
    queue_enqueue(sem->blocked_threads, (void *)tid);
    thread_block();
    exit_critical_section();
    return 0;
  }
}

int sem_up(sem_t sem)
{
  if (!sem)
    return -1;

  enter_critical_section();

  if (queue_length(sem->blocked_threads) == 0)
  {
    sem->count++;
  }
  else
  {
    pthread_t tid;
    queue_dequeue(sem->blocked_threads, (void **)&tid); 
    thread_unblock(tid);
  }
  
  exit_critical_section();
  return 0;

}

int sem_getvalue(sem_t sem, int *sval)
{
  return 0;
}

