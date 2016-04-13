
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
   while(42)
   {
      wait_for_interrupt(TIMER_IRQ);
      send(timer_port, 0);
   }
}

void timer_service(PROCESS self, PARAM param)
{
   PROCESS sender;
   Timer_Message * msg;
   //Create the timer notifier
   create_process(timer_notifier, 
                  7, 
                  0, 
                  "Timer Notifier");  

   //Forever Loop
   while(1)
   {
      msg = (Timer_Message *) receive(&sender);
      if(msg != NULL)
      {
         //register number of ticks client wants to sleep
         int index = sender-pcb;
         ticks_remaining[index] = msg->num_of_ticks;
         continue;
         
      }
      else
      {
         //Message from timer notifier
         int i;
         for(i = 0; i < MAX_PROCS; ++i)
         {
            //decrement their counter
            --ticks_remaining[i];
            if(ticks_remaining[i] == 0)
            {
               reply(&pcb[i]);
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
   resign();
}
