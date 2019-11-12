Project 3 User Level Thread Library Part 2

For the semaphore API, we created a structure of semaphores that contained an
integer count and a queue for blocked threads. The count kept track of how many
resources were currently available, and as long as there was one, we would use
it in sem down and decrease the count. If there was no resource available
(count == 0), we would place the thread in the blocked queue so that it would
receive a resource when one came available and when it was it’s turn. Using a
queue helped us implement the correct scheduling, because it has FIFO
implementation which lets the first blocked thread receive the first available
resource and then the second gets the next and so on. All of this code,
decrementing the count or blocking the thread was done inside a critical
section to ensure that another thread couldn’t take control and take the
resource or enter the queue before the first asking thread. Sem up is similar
but it gives up a resource, and if there are blocked, waiting threads it gives
the now available resource to the top of the queue, the earliest asker.

For the TPS API, we created a structure, TPS, that holds a thread and another
struct inside of it, page, which holds a count and memory address. This was
done so that two (or more) threads can point to the same memory page until one
wants to write something new, i.e. when we would have to copy the address so we
could write and change it.
Tps read and tps write change the permissions using mprotect, which allows us
to initially have no permissions set and then add read and write privileges
only when need be. Write is a little more in depth, because if more than one
thread uses this memory address, we have to create a new page and copy the
contents there so that all other threads can still use the old page. To do
this, we initially give write privileges to a new address, so we don’t have to
instantly change them. We then read the old memory address, changing those
privileges first, and then copy the contents (and alter them) into the new
address with our already initialized writing privileges. At the end we change
everything back to having no privileges and decrement the count of the old
memory address since one less thread is using its’ address.
In tps clone, we iterate on the queue to ensure the tid needing to be cloned 
is indeed a thread and enqueued, and that the current thread does not already
exist, then we set up the current thread with cloned information (initializing
everything the same). This makes the given tid’s count increment as well,
because now two threads (or more depending on how many clones already exist)
use the same memory address.

For testing this threaded library, we had multiple programs ensure that our 
semaphores were implemented correctly. In addition, the tps was tested by tps.c
which tests reading, writing, and cloning. In tps_segfault.c, we test our
implementation in how we handle segfaults. If we have a regular segfault then 
it will show this error, but if we have a segfault because one thread is trying
to access another thread's private storage then it will print out an additional
error message. This allows us to make sure the tps is actuall private storage.
