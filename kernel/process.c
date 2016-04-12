
#include <kernel.h>
#define PROCESS_BASE 640 * 1024
#define PROCESS_SIZE 30 * 1024

PCB pcb[MAX_PROCS];

PORT create_process (void (*ptr_to_new_proc) (PROCESS, PARAM),
		     int prio,
		     PARAM param,
		     char *name)
{
   
   //For interrupts
   volatile int saved_if;
   DISABLE_INTR(saved_if);
   //end for interrupts

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
   new_proc->first_port = NULL;
   new_proc->name       = name;

   PORT p = create_new_port(new_proc);

   esp = PROCESS_BASE - PROCESS_SIZE * i;
   
   poke_l(esp, param);
   esp -= sizeof(LONG);
   poke_l(esp, new_proc);
   esp -= sizeof(LONG);
   poke_l(esp, 0);
   esp -= sizeof(LONG);

   //For interrupts
   if(interrupts_initialized == TRUE)
      poke_l(esp, 512);
   else
      poke_l(esp, 0);
   esp -= sizeof(LONG);

   poke_l(esp, CODE_SELECTOR);
   esp -= sizeof(LONG);
   //End for interrupts

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
   
   //For interrupts
   ENABLE_INTR(saved_if);
   //End for interrupts

   return p;
}


PROCESS fork()
{
    // Dummy return to make gcc happy
    return (PROCESS) NULL;
}




void print_process(WINDOW* wnd, PROCESS p)
{
   const static char * state_list[] = {"READY", 
                          "SEND BLOCKED", 
                          "REPLY BLOCKED", 
                          "RECEIVE BLOCKED",
                          "MESSAGE BLOCKED", 
                          "INTR BLOCKED"};
   char * state;
   switch (p->state){
   case STATE_READY:
      state = state_list[0];
      break;
   case STATE_SEND_BLOCKED:
      state = state_list[1];
      break;
   case STATE_REPLY_BLOCKED:
      state = state_list[2];
      break;
   case STATE_RECEIVE_BLOCKED:
      state = state_list[3];
      break;
   case STATE_MESSAGE_BLOCKED:
      state = state_list[4];
      break;
   case STATE_INTR_BLOCKED:
      state = state_list[5];
      break;
   default:
      assert(0);
      break;
   }
   char * name = p->name;
   char activeLabel = ' ';

   //Asterisk for the active process
   if(active_proc == p)
      activeLabel = '*';
   
   kprintf("%s %10s %c %5s %d %s\n", state, " ", activeLabel, " ",p->priority, name);

}

void print_all_processes(WINDOW* wnd)
{
   //Print out header
   kprintf("%s %10s %s %s %s\n", "State", " ", "Active", "Prio", "Name");

   //Print border
   char border[] = "-------------------------------------------------------\n";
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


