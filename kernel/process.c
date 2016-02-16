
#include <kernel.h>


PCB pcb[MAX_PROCS];
unsigned int numOfProcs;

PORT create_process (void (*ptr_to_new_proc) (PROCESS, PARAM),
		     int prio,
		     PARAM param,
		     char *name)
{
   //Do a check to see if process space available
   if (numOfProcs <= MAX_PROCS)
      return NULL;

   pcb[numOfProcs].magic      = MAGIC_PCB;
   pcb[numOfProcs].used       = TRUE;
   pcb[numOfProcs].state      = STATE_READY;
   pcb[numOfProcs].priority   = prio;
   pcb[numOfProcs].first_port = NULL;
   pcb[numOfProcs].name       = name;

   //Increment the process counter
   numOfProcs++;
  
}


PROCESS fork()
{
    // Dummy return to make gcc happy
    return (PROCESS) NULL;
}




void print_process(WINDOW* wnd, PROCESS p)
{
   char proc_detail[80];
   
   int i;
   for(i = 0; i < 80; ++i){
      proc_detail[80] = (char) 0;
   }

   unsigned short state = p->state;
   switch(state){
      case STATE_READY:
         k_memcpy(proc_detail, "Ready", 7);
       default:
         assert(0);
   }

  // output_string(wnd, proc_detail);
}

void print_all_processes(WINDOW* wnd)
{
   //Print out header
   char header[] = "State             Active Prio Name\n"; 
   output_string(wnd, header);

   char border[] = "-------------------------------------------------";
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
   //Initialize the process counter
   numOfProcs = 0;

   pcb[0].magic      = MAGIC_PCB;
   pcb[0].used       = TRUE;
   pcb[0].state      = STATE_READY;
   pcb[0].priority   = 1;
   pcb[0].first_port = NULL;
   pcb[0].name       = "Boot process";

   //Increment the process counter;
   numOfProcs++;
}



