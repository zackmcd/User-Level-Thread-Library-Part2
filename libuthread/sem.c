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

  if (!sem) // check if sem is NULL
    return NULL;

  sem->count = count;
  sem->blocked_threads = queue_create(); // initialize queue
  
  if (!sem->blocked_threads) // check if queue is NULL
    return NULL;
  
  return sem;
}

int sem_destroy(sem_t sem)
{
  if (!sem)
    return -1;
  
  free(sem); // frees sem
  return 0;
}

int sem_down(sem_t sem)
{
  if (!sem)
    return -1;

  enter_critical_section(); // enters critical section

  if (sem->count > 0) // there is a resource to be used
  {
    sem->count--; // taking a resource away
    exit_critical_section();
    return 0;
  }
  else // if there are no more resources available and we must block the thread
  {
    pthread_t tid = pthread_self();
    queue_enqueue(sem->blocked_threads, (void *)tid); // adds thread to queue of blocked threads
    thread_block(); // this actually blocks thread
    exit_critical_section();
    return 0;
  }
}

int sem_up(sem_t sem)
{
  if (!sem)
    return -1;

  enter_critical_section(); // enters critical section

  if (queue_length(sem->blocked_threads) == 0) // if there are no waiting threads
  {
    sem->count++; // increases the amount of available resources
  }
  else // if there are waiting threads go and get the next in line
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
  if (!sem || !sval)
    return -1;

  if (sem->count > 0)
  {
    *sval = sem->count; // returns number of resources available
  }
  else if (sem->count == 0) // returns the negative number of how many waiting threads
  {
    *sval = -1 * queue_length(sem->blocked_threads);
  }
  
  return 0;
}

