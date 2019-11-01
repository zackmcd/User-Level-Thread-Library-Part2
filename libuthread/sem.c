#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "thread.h"

struct semaphore {
  size_t count;
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
	/* TODO Phase 1 */
}

int sem_up(sem_t sem)
{
	/* TODO Phase 1 */
}

int sem_getvalue(sem_t sem, int *sval)
{
	/* TODO Phase 1 */
}

