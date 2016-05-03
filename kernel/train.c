#include <kernel.h> 
#define DEFAULT_SLEEP_TICK 15
#define ZAMBONI_COUNTER_CLOCKWISE 1
#define ZAMBONI_CLOCKWISE 2

#define CONFIG_1 0x01
#define CONFIG_2 0x02
#define CONFIG_3 0x03
#define CONFIG_4 0x04

#define RESET "R\015"
#define REAL_TRAIN 0
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

/*
   Function pointer to run a certain train configuration
*/

void (*config_func)();

//**************************
//run the train application
//**************************
/*
   Main API to send message to train hardware
*/
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

/*
   Function to poll a specified track at num_poll times
*/
void poll_track(char * buffer, 
                const unsigned int buf_length, 
                int num_poll,
                char * track)
{
   int i = 0;
   while(i < num_poll)
   {
      send_message_to_train(RESET, NULL, 0, DEFAULT_SLEEP_TICK);
      send_message_to_train(track, buffer, 3, DEFAULT_SLEEP_TICK);
      if(buffer[1] == '1')
         break;
      i++;
   }
}

/*
   Send a series of commands in rapid succession that do not require response
*/
void send_sequential_commands(char ** command_list, int list_length)
{
   int i;
   for(i = 0; i < list_length; i++)
   {
      send_message_to_train(command_list[i], NULL, 0, DEFAULT_SLEEP_TICK);
   }
}

/*
   Function to stop train and change direction
*/

/************** >>> Run the different configurations <<< ************/
void run_config_1()
{
   //Response buffer
   char buffer[3] = {' ', ' ', ' '};

   //Move the train
   send_message_to_train("L20S5\015", NULL, 0, DEFAULT_SLEEP_TICK);

//If this is a real train slow the train down a bit
#if REAL_TRAIN
   send_message_to_train("L20S4\015", NULL, 0, DEFAULT_SLEEP_TICK);
#endif

   //Poll track 6 to see if train is there
   poll_track(buffer, 3, 15, "C6\015");
   char * commands[] = {"M4R\015", "M3G\015", "L20S4\015"};
   if(buffer[1] == '1')
      send_sequential_commands(commands, 3);

   //Clear buffer
   buffer[0] = buffer[1] = buffer[2] = ' ';

   //Poll track 1 to ensure that train and wagon are together
   poll_track(buffer, 3, 15, "C1\015");

   //Prepare command list if train is there
   char * commands2[] = {"L20S0\015", "L20D\015", "M5R\015", "M6R\015", "L20S5\015"};
   if(buffer[1] == '1')
      send_sequential_commands(commands2, 5);

   //Clear buffer
   buffer[0] = buffer[1] = buffer[2] = ' ';

   //Poll track 8 to stop train
   poll_track(buffer, 3, 20, "C8\015");
   if(buffer[1] == '1')
      send_message_to_train("L20S0\015", NULL, 0, DEFAULT_SLEEP_TICK);
}

void run_config_2()
{
   run_config_1();
}

void run_config_3()
{
   //Response buffer
   char buffer[3] = {' ', ' ', ' '};

   //Start the train
   send_message_to_train("L20S5\015", NULL, 0, DEFAULT_SLEEP_TICK);

   //Poll track 13 to prepare tracks
   poll_track(buffer, 3, 15, "C13\015");

   char * commands[] = {"M1R\015", "M2R\015", "M7R\015"};
   if(buffer[1] == '1')
      send_sequential_commands(commands, 3);

   //Clear buffer
   buffer[0] = buffer[1] = buffer[2] = ' ';

   //Poll track 13 to ensure train and wagon are together is there 
   poll_track(buffer, 3, 15, "C13\015");

   //Change 1 to green lane temporarily
   if(buffer[1] == '1')
      send_message_to_train("M1G\015", NULL, 0, DEFAULT_SLEEP_TICK);

   //Clear buffer
   buffer[0] = buffer[1] = buffer[2] = ' ';

   //Poll track 3 to close switch 1
   poll_track(buffer, 3, 10, "C3\015");
   if(buffer[1] == '1')
      send_message_to_train("M1R\015", NULL, 0, DEFAULT_SLEEP_TICK);

   //Clear buffer
   buffer[0] = buffer[1] = buffer[2] = ' ';

   //Poll track 6
   poll_track(buffer, 3, 10, "C6\015");
   char * commands2[] = {"L20S0\015", "M4R\015", "M3R\015", "L20D\015", "L20S4\015"};
   if(buffer[1] == '1')
      send_sequential_commands(commands2, 5);

   //Clear buffer
   buffer[0] = buffer[1] = buffer[2] = ' ';

   //Poll track 5 to stop train
   poll_track(buffer, 3, 10, "C5\015");
   if(buffer[1] == '1')
      send_message_to_train("L20S0\015", NULL, 0, DEFAULT_SLEEP_TICK);
}

void run_config_4()
{
   //Response buffer
   char buffer[3] = {' ', ' ', ' '};

   //Start the train
   send_message_to_train("L20S5\015", NULL, 0, DEFAULT_SLEEP_TICK);

   //Poll track 6
   poll_track(buffer, 3, 10, "C6\015");
   char * commands[] = {"L20S0\015", "M4G\015", "L20D\015", "L20S5\015"};
   if(buffer[1] == '1')
      send_sequential_commands(commands, 4);

   //Clear buffer
   buffer[0] = buffer[1] = buffer[2] = ' ';

   //Poll track 14
   poll_track(buffer, 3, 15, "C14\015");
   char * commands2[] = {"L20S0\015", "M9G\015", "L20D\015", "L20S5\015"};
   if(buffer[1] == '1')
      send_sequential_commands(commands2, 4);

   //No deterministic way so just sleep and hope train makes contact
   sleep(DEFAULT_SLEEP_TICK * 18);

   //Stop the train
   send_message_to_train("L20S0\015", NULL, 0, DEFAULT_SLEEP_TICK);

   //Ensure train stops
   sleep(DEFAULT_SLEEP_TICK);
   
   //Change direction
   send_message_to_train("L20D\015", NULL, 0, DEFAULT_SLEEP_TICK);

   //Bring train back home
   char * commands3[] = {"L20S5\015", "M8G\015"};
   send_sequential_commands(commands3, 2);

   //Clear buffer
   buffer[0] = buffer[1] = buffer[2] = ' ';

   //Poll track 10 to close up the right loop
   poll_track(buffer, 3, 15, "C10\015");
   if(buffer[1] == '1')
      send_message_to_train("M8R\015", NULL, 0, DEFAULT_SLEEP_TICK);


   //Clear buffer
   buffer[0] = buffer[1] = buffer[2] = ' ';

   //poll track 7
   poll_track(buffer, 3, 15, "C7\015");
   char * commands4[] = {"M4R\015", "M3R\015"};
   if(buffer[1] == '1')
      send_sequential_commands(commands4, 2);


  //Poll track 5
  poll_track(buffer, 3, 10, "C5\015");
  if(buffer[1] == '1')
   send_message_to_train("L20S0\015", NULL, 0, DEFAULT_SLEEP_TICK);
}

/*
   Set up the initial tracks
*/
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
   send_sequential_commands(track_list, 5);
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
   int finished = FALSE;

   //Title
   output_string(&train_wnd, "Welcome to Train Simulator\n");

   //Enumerated type
   ZAMBONI_DIRECTION = no_zamboni;
   
   //Create one big loop for zamboni
   set_initial_tracks();

   //Determine train/wagon config
   determine_train_config();

   //Print out the train config on the window
   switch(train_config)
   {
      case CONFIG_1:
         output_string(&train_wnd, "Configuration 1/2\n");
         config_func = run_config_1;
         break;
      case CONFIG_3:
         output_string(&train_wnd, "Configuration 3\n");
         config_func = run_config_3;
         break;
      case CONFIG_4:
         output_string(&train_wnd, "Configuration 4\n");
         config_func = run_config_4;
         break;
      default:
         output_string(&train_wnd, "Could not determine train/wagon configuration\n");
         break;
   }

   while(1)
   {
      sleep(15);

      if(!finished)
      {
         (*config_func)();
         finished = TRUE;
      }
      else
      {
         
      }
   }
}


void init_train(WINDOW* wnd)
{
   create_process(train_process, 
                  3, 
                  0, 
                  "Train Process");

   resign();
}

