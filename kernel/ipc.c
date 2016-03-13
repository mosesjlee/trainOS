
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
         break;
      }
   }
   if((*owner)->first_port == NULL)

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
   assert (dest_port->owner->magic == MAGIC_PCB && dest_port->magic == MAGIC_PORT);

   if(dest_port->owner->state == STATE_RECEIVE_BLOCKED && 
      dest_port->open == TRUE)
   {

      //The process receiving message is ready
      dest_port->owner->state = STATE_READY;

      //Pass the data pointer
      dest_port->owner->param_data   = data;

      //Set the param_proc
      dest_port->owner->param_proc   = active_proc;

      //Set current process to reply blocked
      active_proc->state      = STATE_REPLY_BLOCKED;

      //Add back to queue
      add_ready_queue(dest_port->owner);
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

      active_proc->param_data = data;
      active_proc->state = STATE_SEND_BLOCKED;

   }

   remove_ready_queue(active_proc);
   resign();
}


void message (PORT dest_port, void* data)
{
   if(dest_port->owner->state == STATE_RECEIVE_BLOCKED &&
      dest_port->open == TRUE)
   {  
      dest_port->owner->state = STATE_READY;
      dest_port->owner->param_data = data;
      dest_port->owner->param_proc = active_proc;
      add_ready_queue(dest_port->owner);
   }
   else
   {
       //If first element on that list
      if(dest_port->blocked_list_head == NULL &&
         dest_port->blocked_list_tail == NULL)
      {
         dest_port->blocked_list_head = active_proc;
         dest_port->blocked_list_tail = active_proc;
         active_proc->next_blocked = NULL; 
      }
      //If there are elements on that list
      else
      {
         dest_port->blocked_list_tail->next_blocked = active_proc;
         dest_port->blocked_list_tail = active_proc;
         active_proc->next_blocked = NULL;
      }
      active_proc->param_data = data;
      active_proc->param_proc = dest_port->owner;
      active_proc->state = STATE_MESSAGE_BLOCKED;
      remove_ready_queue(active_proc);
   }
   resign();
}



void* receive (PROCESS* sender)
{
   *sender = NULL;
   PORT p = active_proc->first_port;
   while(p != NULL){
      if(p->open == TRUE && p->blocked_list_head != NULL){
            *sender = p->blocked_list_head;
            p->blocked_list_head = (*sender)->next_blocked;
            break;
      }
      p = p->next;
   }
  
   if(*sender != NULL)
   {
      assert((*sender)->magic == MAGIC_PCB);

      if((*sender)->state == STATE_MESSAGE_BLOCKED)
      {
         (*sender)->state = STATE_READY;
         add_ready_queue((*sender));
      }
      else if((*sender)->state == STATE_SEND_BLOCKED)
      {
         (*sender)->state = STATE_REPLY_BLOCKED;
      }
      
   }

   else
   {
      //No senders get off queue
      active_proc->state = STATE_RECEIVE_BLOCKED;
      remove_ready_queue(active_proc);
      resign();
      *sender = active_proc->param_proc;
      (*sender)->param_data = active_proc->param_data;
   }

   return (*sender)->param_data;
}


void reply (PROCESS sender)
{
   assert(sender->state == STATE_REPLY_BLOCKED);
   add_ready_queue(sender);
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
