#include <kernel.h> 
#define DEFAULT_SLEEP_TICK 15
#define ZAMBONI_COUNTER_CLOCKWISE 1
#define ZAMBONI_CLOCKWISE 2

#define CONFIG_1 0x01
#define CONFIG_2 0x02
#define CONFIG_3 0x03
#define CONFIG_4 0x04

#define RESET "R\015"

/*
  Train window
*/
WINDOW train_wnd = {0, 0, 80, 8, 0, 0, ' '}; 

/*
  Zamboni variables
*/
int zamboni = FALSE;
unsigned int zamboni_direction = 0;

enum _zamboni_direction {
   no_zamboni,
   counter_clockwise,
   clockwise
} ZAMBONI_DIRECTION;


/*
  Train and wagon configurations
*/
unsigned int train_config = 0;

const static char * train_locs [] = {"C5\015", "C8\015"};
const static char * wagon_locs [] = {"C2\015", "C11\015", "C16\015"};

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

void send_message_to_train(const char * command, 
                           char * response, 
                           const unsigned int response_length,
                           const unsigned int sleep_time)
{
   //Ensure 15 tick interval
   sleep(sleep_time);
   
   COM_Message msg;
   msg.output_buffer = command;
   msg.input_buffer = response;
   msg.len_input_buffer = response_length;

   //Send the message
   send(com_port, &msg);
}

void set_initial_tracks()
{
   static char * track_list [] = {
                                  "M4G\015",
                                  "M5G\015",
                                  "M8G\015",
                                  "M9R\015",
                                  "M1G\015",
                                 };

   //Set up the initial track for one big loop
   int i;
   for(i = 0; i < 5; i++)
      send_message_to_train(track_list[i], NULL, 0, DEFAULT_SLEEP_TICK);
}

void determine_zamboni_config()
{
   char response[8];
   char reset[] = RESET;
   char * directions[] = {
                          "C3\015",
                          "C4\015",
                          "C6\015",
                          "C7\015",
                          "C10\015",
                          "C13\015",
                          "C14\015",
                          "C15\015",
                         };

   int i;
   for(i = 0; i < 8; i++)
   {
      send_message_to_train(reset, response, 0, DEFAULT_SLEEP_TICK);
      send_message_to_train(directions[i], response, 3, 90);
   }
}

void detect_zamboni_presence()
{
   char response[4];
   send_message_to_train(RESET, response, 0, DEFAULT_SLEEP_TICK);
   send_message_to_train("C4\015", response, 3, DEFAULT_SLEEP_TICK);

   if(response[1] == '1')
   {
      zamboni = TRUE;
   }
}

void determine_train_config()
{   
   //Buffer for response
   char response[4];

   //Train and wagon locations
   int train = -1;
   int wagon = -1;

   //Look for train
   int i;
   for(i = 0; i < 2; i++)
   {
      send_message_to_train(RESET, response, 0, DEFAULT_SLEEP_TICK);
      send_message_to_train(train_locs[i], response, 3, DEFAULT_SLEEP_TICK);

      //Determine if train was found
      if(response[1] == '1')
      {
         train = i;
         break;
      }
   }

   //Look for wagon
   for(i = 0; i < 3; i++)
   {
      send_message_to_train(RESET, response, 0, DEFAULT_SLEEP_TICK);
      send_message_to_train(wagon_locs[i], response, 3, DEFAULT_SLEEP_TICK);

      //Determine if wagon was found
      if(response[1] == '1')
      {
         wagon = i;
         break;
      }
   }

   //Determine train/wagon configuration
   if(train == 0)
   {
      if(wagon == 1)
         train_config = CONFIG_3;
      else if(wagon == 2)
         train_config = CONFIG_4;
      else
         output_string(&train_wnd, "Unable to locate wagon location\n");
   }
   else if(train == 1)
   {
      if(wagon == 0)
         train_config = CONFIG_1;
      else
         output_string(&train_wnd, "Unable to locate wagon location\n");
   }
   else
   {
      output_string(&train_wnd, "Unable to locate train/wagon location\n");
   }
}

//Main Train process
void train_process(PROCESS self, PARAM param)
{
   //Title
   output_string(&train_wnd, "Welcome to Train Simulator\n");

   //Enumerated type
   ZAMBONI_DIRECTION = no_zamboni;
   
   //Determine if zamboni exists
   detect_zamboni_presence();

   //Create one big loop for zamboni
   set_initial_tracks();

   //Determine train/wagon config
   determine_train_config();

   //Print out the train config on the window
   if(train_config == CONFIG_1)
   {
      output_string(&train_wnd, "Configuration 1/2\n");
   }
   else if(train_config == CONFIG_3)
   {
      output_string(&train_wnd, "Configuration 3\n");
   }
   else if(train_config == CONFIG_4)
   {
      output_string(&train_wnd, "Configuration 4\n");
   }
   else
   {
      output_string(&train_wnd, "Could not determine train/wagon configuration\n");
   }

   while(1)
   {
      if(zamboni)
         determine_zamboni_config();
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

