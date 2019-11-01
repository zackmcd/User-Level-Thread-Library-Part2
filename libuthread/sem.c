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

  // disable interrupts

  if (sem->count >= 0)
  {
    sem->count--;
    //enable interrupts
    return 0;
  }

  //enqueue(sem->blocked_threads, //current thread )
  //enable interrupts
  return 0; // maybe not return 0
}

int sem_up(sem_t sem)
{
  if (!sem)
    return -1;

  //disbale interrupts
  if (queue_length(sem->blocked_threads) == 0)
  {
    sem->count++;
  }
  else
  {
    // temp thread = dequeue(sem->blocked_threads);
    // unblock thread with tid
  }
  //enable interrupts
}

int sem_getvalue(sem_t sem, int *sval)
{
	/* TODO Phase 1 */
}

