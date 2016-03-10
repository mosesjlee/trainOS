
#include <kernel.h>


PORT_DEF ports[MAX_PORTS];

//Helper function
PORT allocate_port(PROCESS * owner)
{

   assert((*owner)->magic == MAGIC_PCB);
   int i;
   for(i = 0; i < MAX_PORTS; ++i){
      if(ports[i].owner == NULL && ports[i].used == FALSE){
         //Assert the value
         assert(ports[i].magic == MAGIC_PORT);
         ports[i].owner = *owner;
         ports[i].used  = TRUE;
         ports[i].open  = TRUE;
         (*owner)->first_port = &ports[i];
         break;
      }
   }

   return &ports[i];
}

PORT create_port()
{
   return allocate_port(&active_proc);
}


PORT create_new_port (PROCESS owner)
{
   return allocate_port(&owner);
}


void open_port (PORT port)
{
   port->open = TRUE;
}



void close_port (PORT port)
{
   port->open = FALSE;
}


void add_to_blocked_list()
{
}

void send (PORT dest_port, void* data)
{
   if(dest_port->owner->state == STATE_RECEIVE_BLOCKED && 
      dest_port->open == TRUE)
   {
      //The process receiving message is ready
      dest_port->owner->state = STATE_READY;

      //Pass the data pointer
      dest_port->owner->param_data   = data;

      //Set current process to reply blocked
      active_proc->state      = STATE_REPLY_BLOCKED;
   }
   else
   {
      //If first element on that list
      if(dest_port->blocked_list_head == NULL &&
         dest_port->blocked_list_tail == NULL)
      {
         dest_port->blocked_list_head = active_proc;
         dest_port->blocked_list_tail = active_proc;
         active_proc->next_blocked = active_proc; 
      }
      //If there are elements on that list
      else
      {
         dest_port->blocked_list_tail->next_blocked = active_proc;
         dest_port->blocked_list_tail = active_proc;
         active_proc->next_blocked = dest_port->blocked_list_head;
      }

      active_proc->state = STATE_SEND_BLOCKED;
   }
}


void message (PORT dest_port, void* data)
{
   if(active_proc->state == STATE_RECEIVE_BLOCKED &&
      dest_port->open == TRUE)
   {
      dest_port->owner->state = STATE_READY;
      dest_port->owner->param_data  = data;
   }
   else
   {
       //If first element on that list
      if(dest_port->blocked_list_head == NULL &&
         dest_port->blocked_list_tail == NULL)
      {
         dest_port->blocked_list_head = active_proc;
         dest_port->blocked_list_tail = active_proc;
         active_proc->next_blocked = active_proc; 
      }
      //If there are elements on that list
      else
      {
         dest_port->blocked_list_tail->next_blocked = active_proc;
         dest_port->blocked_list_tail = active_proc;
         active_proc->next_blocked = dest_port->blocked_list_head;
      }

      active_proc->state = STATE_MESSAGE_BLOCKED;
   }
}



void* receive (PROCESS* sender)
{
   PROCESS p = *sender;
   if(p->first_port->blocked_list_head != NULL)
   {
      PROCESS s = p->first_port->blocked_list_head;
      if(s->state == STATE_MESSAGE_BLOCKED)
      {
      }
      if(s->state == STATE_SEND_BLOCKED)
      {
      }
   }
   else
   {
   }
}


void reply (PROCESS sender)
{
   resign();
}


void init_ipc()
{
   int i;
   for(i = 0; i < MAX_PORTS; ++i)
   {
      ports[i].magic = MAGIC_PORT;
      ports[i].used  = FALSE;
      ports[i].open  = TRUE;
      ports[i].owner = NULL;
      ports[i].blocked_list_head = ports[i].blocked_list_tail = NULL;
      ports[i].next  = NULL;
      
   }
}
