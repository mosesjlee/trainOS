
#include <kernel.h>


PORT timer_port;

int ticks_remaining[MAX_PROCS];


void sleep(int ticks)
{
   Timer_Message msg;
   msg.num_of_ticks = ticks;
   send(timer_port, &msg);
}

void timer_notifier(PROCESS self, PARAM param)
{
   Timer_Message msg = {0};
   while(42)
   {
      wait_for_interrupt(TIMER_IRQ);
      send(timer_port, &msg);
   }
}

void timer_service(PROCESS self, PARAM param)
{
   //Create the timer notifier
   create_process(timer_notifier, 
                  7, 
                  0, 
                  "Timer Notifier");  

   //Forever Loop
   while(1)
   {
      Timer_Message * msg = (Timer_Message *) receive(timer_port->blocked_list_head);

      if(timer_port->blocked_list_head != timer_notifier)
      {
         //register number of ticks client wants to sleep
         
      }
      else
      {
         //Message from timer notifier
         int i;
         for(i = 0; i < MAX_PROCS; ++i)
         {
            //decrement their counter
            --ticks_remaining[i];
            if(ticks_remaining[i] <= 0)
            {
               reply(pcb[i]);
               ticks_remaining[i] = 0;
            }
         }
      }
   }
}

void init_timer ()
{
   timer_port = create_process(timer_service, 
                               6,
                               0,
                              "Timer Service");
   int i;
   for(i = 0; i < MAX_PROCS; ++i)
   {
      ticks_remaining[i] = 0;
   }
}
