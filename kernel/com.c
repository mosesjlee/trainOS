/* 
 * Internet ressources:
 * 
 * http://workforce.cup.edu/little/serial.html
 *
 * http://www.lammertbies.nl/comm/info/RS-232.html
 *
 */


#include <kernel.h>

PORT com_port;
PORT com_reader_port;

void com1_example()
{
   char buffer[12];
   COM_Message msg;
   int i;

   msg.output_buffer = "Hello World!";
   msg.input_buffer = buffer;
   msg.len_input_buffer = 12;

   send(com_port, &msg);
   for(i = 0; i < 12; i++)
      kprintf("%c", buffer[i]);
}

void com_reader_process(PROCESS self, PARAM param);

void com_process(PROCESS self, PARAM param)
{
   PROCESS sender;
   PROCESS recv_proc;
   COM_Message * msg;

   com_reader_port = create_process(com_reader_process, 
                                    7,
                                    (PARAM) self->first_port,
                                    "Com Reader Process");

   while(1){
      //receive message from user process
      msg = (COM_Message *) receive(&sender);

      //forward message to COM reader process
      message(com_reader_port, msg);

      //write all bytes contained in COM_Message.output_buffer to COM1
      char * c = msg->output_buffer;
      while(*c != '\0')
      {
         while(!(inportb(COM1_PORT + 5) & (1 << 5)));
         outportb(COM1_PORT, *c);
         c++;
      }

      //wait for message from COM reader process that signals that all bytes have been read
      receive(&recv_proc); 

      //reply to user process to signal that all I/O has been completed
      reply(sender);

   }
}

void com_reader_process(PROCESS self, PARAM param)
{
   PROCESS sender;
   PORT reply_port = (PORT) param;
   COM_Message * msg;

   while(1)
   {
      //receive message from COM_Process
      msg = (COM_Message *) receive(&sender);

      //Message contains number of bytes to read into COM_Message.len_input_buffer
      int i;

      //read as many bytes requested from COM1 using wait_for_interrupt(COM1_IRQ) 
      //and importb(COM1_PORT)

      for(i = 0; i < msg->len_input_buffer; i++){
         wait_for_interrupt(COM1_IRQ);
         msg->input_buffer[i] = inportb(COM1_PORT);
      }
      //send message to COM Process to signal that all bytes have been read
      message(reply_port, &msg);
   }
}

void init_uart()
{
    /* LineControl disabled to set baud rate */
    outportb (COM1_PORT + 3, 0x80);
    /* lower byte of baud rate */
    outportb (COM1_PORT + 0, 0x30);
    /* upper byte of baud rate */
    outportb (COM1_PORT + 1, 0x00);
    /* 8 Bits, No Parity, 2 stop bits */
    outportb (COM1_PORT + 3, 0x07);
    /* Interrupt enable*/
    outportb (COM1_PORT + 1, 1);
    /* Modem control */
    outportb (COM1_PORT + 4, 0x0b);
    inportb (COM1_PORT);
}



void init_com ()
{
   init_uart();
   com_port = create_process(com_process,
                             6,
                             0,
                             "COM process");
   resign();
}
