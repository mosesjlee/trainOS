#include <kernel.h> 
#define SLEEP_TICK 15

WINDOW train_wnd = {0, 0, 80, 8, 0, 0, ' '}; 

int zamboni = FALSE;
int direction = FALSE;

//**************************
//run the train application
//**************************

/************** >>> Run the different configurations <<< ************/
void run_config_1()
{
   static char * commands [] = {
                                "M4R\015",
                                "M3G\015",
                                "L20S5\015",
                                ""
                               };
   COM_Message msg;
}

void run_config_2()
{
}

void run_config_3()
{
}

void run_config_4()
{
}

void determine_config()
{
   COM_Message msg;
   char response[12];
   char reset[] = "R\015";
   char check4[] = "C4\015";

   msg.input_buffer = response; msg.len_input_buffer = 0;

   msg.output_buffer = reset;
   send(com_port, &msg);

   sleep(SLEEP_TICK);

   msg.output_buffer = check4;
   msg.len_input_buffer = 3;
   send(com_port, &msg);

   if(msg.input_buffer[1] == '0')
      output_string(&train_wnd, "                         No Zamboni\n");
         
   sleep(SLEEP_TICK);
   msg.output_buffer = "R\015";
   msg.len_input_buffer = 0;
   send(com_port, &msg);

   sleep(SLEEP_TICK);
   msg.output_buffer = "C6\015";
   msg.len_input_buffer = 3;
   send(com_port, &msg);


   sleep(SLEEP_TICK);
   msg.output_buffer = "R\015";
   msg.len_input_buffer = 0;
   send(com_port, &msg);

   sleep(SLEEP_TICK);
   msg.len_input_buffer = 3;
   msg.output_buffer = "C3\015";
   send(com_port, &msg);
}

//Main Train process
void train_process(PROCESS self, PARAM param)
{
   static char * locations[] = {
                              "C8\015",
                              "C2\015",
                              "C5\015",
                              "C11\015",
                              "C5\015",
                              "C16\015"
                              };

   output_string(&train_wnd, "Welcome to Train Simulator\n");

   while(1)
   {
      determine_config();
      sleep(SLEEP_TICK);
   }
}


void init_train(WINDOW* wnd)
{
   create_process(train_process, 
                  5, 
                  0, 
                  "Train Process");

   resign();
}

