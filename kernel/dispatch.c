
#include <kernel.h>

#include "disptable.c"


PROCESS active_proc;


/*
 * Ready queues for all eight priorities.
 */
PCB *ready_queue [MAX_READY_QUEUES];

/*
 * add_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by p is put the ready queue.
 * The appropiate ready queue is determined by p->priority.
 */

void add_ready_queue (PROCESS proc)
{
   //Assert
   assert(proc->magic == MAGIC_PCB);

   //Set it to ready
   proc->state = STATE_READY;

   //Pointer to a pointer
   PROCESS * p = &ready_queue[proc->priority];

   //If there are no processes at that priority
   if(ready_queue[proc->priority] == NULL){
      *p = proc;
      proc->prev = proc;
      proc->next = proc;
   }
   //If there are/is process(es)
   else {
      proc->next = *p;
      proc->prev = (*p)->prev;
      (*p)->prev->next = proc;
      (*p)->prev = proc;
   }
}


/*
 * remove_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by p is dequeued from the ready
 * queue.
 */

void remove_ready_queue (PROCESS proc)
{
   //Assert
   assert(proc->magic == MAGIC_PCB);

   //If there is only one process all the pointers should point to itself
   if(ready_queue[proc->priority]->next == proc && ready_queue[proc->priority] == proc) {
      ready_queue[proc->priority] = NULL;
   }
   //For all other scenarios
   else {
      PROCESS n = proc->next;
      proc->prev->next = n;
      n->prev = proc->prev;
      ready_queue[proc->priority] = n;
   }
}



/*
 * dispatcher
 *----------------------------------------------------------------------------
 * Determines a new process to be dispatched. The process
 * with the highest priority is taken. Within one priority
 * level round robin is used.
 */

PROCESS dispatcher()
{

   int i;
   for(i = MAX_READY_QUEUES - 1; i >= 0; i--){
      if(ready_queue[i] != NULL){
         PROCESS p = ready_queue[i];
         ready_queue[i] = p->next;
         return p;
      }
   }
}

void check_active(){
   assert(active_proc != NULL);
}

/*
 * resign
 *----------------------------------------------------------------------------
 * The current process gives up the CPU voluntarily. The
 * next running process is determined via dispatcher().
 * The stack of the calling process is setup such that it
 * looks like an interrupt.
 */
void resign()
{
   asm("pushl %edi");
   asm("pushl %esi");
   asm("pushl %ebp");
   asm("pushl %ebx");
   asm("pushl %edx");
   asm("pushl %ecx");
   asm("pushl %eax");

   asm("movl %%esp, %0" : "=r" (active_proc->esp) :);
   active_proc = dispatcher();
   check_active();
   asm("movl %0, %%esp" : : "r" (active_proc->esp));
   
   asm("popl %eax");
   asm("popl %ecx");
   asm("popl %edx");
   asm("popl %ebx");
   asm("popl %ebp");
   asm("popl %esi");
   asm("popl %edi");
   asm("ret");
}



/*
 * init_dispatcher
 *----------------------------------------------------------------------------
 * Initializes the necessary data structures.
 */

void init_dispatcher()
{
   //null out the pointers
   int i;
   for(i = 0; i < MAX_READY_QUEUES; ++i){
      ready_queue[i] = NULL;
   }

   //Adds the boot process to the ready queue
   add_ready_queue(active_proc);
}
