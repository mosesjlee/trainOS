
#include <kernel.h>
#define PROCESS_BASE 640 * 1024
#define PROCESS_SIZE 30 * 1024

PCB pcb[MAX_PROCS];

PORT create_process (void (*ptr_to_new_proc) (PROCESS, PARAM),
		     int prio,
		     PARAM param,
		     char *name)
{
   //Look for empty
   int i;
   for(i = 0; i < MAX_PROCS; i++){
      if(pcb[i].used == FALSE)
         break;
   }

   MEM_ADDR esp;
   PROCESS new_proc = &pcb[i];
   
   new_proc->magic      = MAGIC_PCB;
   new_proc->used       = TRUE;
   new_proc->state      = STATE_READY;
   new_proc->priority   = prio;
   new_proc->first_port = create_new_port(new_proc);
   new_proc->name       = name;

   esp = PROCESS_BASE - PROCESS_SIZE * i;

   poke_l(esp, param);
   esp -= sizeof(LONG);
   poke_l(esp, new_proc);
   esp -= sizeof(LONG);
   poke_l(esp, 0);
   esp -= sizeof(LONG);
   poke_l(esp, ptr_to_new_proc);
   esp -= sizeof(LONG);
   
   //0 out register 
   poke_l(esp, 0);         //EAX
   esp -= sizeof(LONG);
   poke_l(esp, 0);         //ECX
   esp -= sizeof(LONG);
   poke_l(esp, 0);         //EDX
   esp -= sizeof(LONG);
   poke_l(esp, 0);         //EBX
   esp -= sizeof(LONG);
   poke_l(esp, 0);         //EBP
   esp -= sizeof(LONG);
   poke_l(esp, 0);         //ESI
   esp -= sizeof(LONG);
   poke_l(esp, 0);         //EDI

   //Save the Stack pointer
   new_proc->esp = esp;

   add_ready_queue(new_proc);
   
   return new_proc->first_port;
}


PROCESS fork()
{
    // Dummy return to make gcc happy
    return (PROCESS) NULL;
}




void print_process(WINDOW* wnd, PROCESS p)
{
   char ready[] = "READY";
   char * name = p->name;
   char activeLabel = ' ';

   //Asterisk for the active process
   if(active_proc == p)
      activeLabel = '*';
   
   kprintf("%s %10s %c %5s %d %s\n", ready, " ", activeLabel, " ",p->priority, name);

}

void print_all_processes(WINDOW* wnd)
{
   //Print out header
   kprintf("%s %10s %s %s %s\n", "State", " ", "Active", "Prio", "Name");

   //Print border
   char border[] = "-------------------------------------------------\n";
   output_string(wnd, border);

   int i;
   for(i = 0; i < MAX_PROCS; i++){
      if(pcb[i].used == TRUE){
         print_process(wnd, &pcb[i]);
      }
   }
}



void init_process()
{
   int i;
   for(i = 0; i < MAX_PROCS; i++){
      pcb[i].used = FALSE;
      pcb[i].magic = 0;
   }

   pcb[0].magic      = MAGIC_PCB;
   pcb[0].used       = TRUE;
   pcb[0].state      = STATE_READY;
   pcb[0].priority   = 1;
   pcb[0].first_port = NULL;
   pcb[0].name       = "Boot process";

   active_proc = &pcb[0];
}


