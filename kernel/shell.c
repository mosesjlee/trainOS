#include <kernel.h>

#define ENTER_COM     0x0D
#define BACKSPACE_COM 0x08
#define MAX_BUF_COM   60
#define MAX_FLAG_CHAR 16
#define DEBUG 0
#define TURN_IN_MODE 0

WINDOW shell_wnd  = {0, 9, 61, 16, 0, 0, 0xDC};
WINDOW pac_wnd = {61, 9, 19, 16, 0, 0, 0xDC};
extern WINDOW train_wnd;

//Helper debug function
/*****>>> Below are my helper functions <<<********************/
void print_current_cmd(char * cmd, int count)
{
   int i;
   for(i = 0; i < count; i++)
   {
      kprintf("%c", cmd[i]);
   }
   kprintf("\n");
}

/*
Helper functio to clear out character buffer
*/
void clear_buf(char * com, int char_count)
{
   int i;
   for(i = 0; i < char_count; i++)
      com[i] = (char) 0;
}

//Modifies the character buffer as needed
void modify_buffer_at_index(char c, char * buf, int index)
{
   //If index is greater or below the buffer count, just return
   if(index > MAX_BUF_COM - 1 || index < 0) return;

   buf[index] = c;
}

//I included this in my own stdlib.c, but for turn in include these for compiling
#if TURN_IN_MODE
//My own string compare
int k_strcmp(const char * s1, const char *s2)
{
   while(*s1 != '\0' && *s2 != '\0')
   {
      int d = *s1++ - *s2++;
      if(d != 0) return (d);
   }

   if(*s1 == '\0' && *s2 == '\0')
      return 0;
   else
      return *s1 - *s2;
}

//My own atoi
int k_atoi(const char * s)
{
   int res = 0;
   int negative = 1;
   if(s[0] == '-') 
   {
      negative = -1;
      s++;
   }
   while(*s != '\0')
   {
      if(*s < '0' || *s > '9') return res * negative;
      res = res * 10 + *s++ - '0';
   }

   return res * negative;
}
#endif


/********>>>>>>>>>>Above are my helper functions <<<<<<<<<<<<<<<<*********/


//Function that prints out all the help options
void print_help_menu(const char * flags)
{
   output_string(&shell_wnd, "List of Available Commands\n");
   output_string(&shell_wnd, "help    ------- Displays help menu\n");
   output_string(&shell_wnd, "clear   ------- Clears window\n");
   output_string(&shell_wnd, "about   ------- Credits and License\n");
   output_string(&shell_wnd, "ps      ------- Print current processes\n");
   output_string(&shell_wnd, "pacman  ------- Runs pacman game. Pass -h for more options\n");
   output_string(&shell_wnd, "train   ------- Runs train app. Pass -h for more options\n");
}

//Prints credits
void print_credits(const char * flags)
{
   output_string(&shell_wnd, "***************  TOS  ******************\n");
   output_string(&shell_wnd, "Train Operating System\n");
   output_string(&shell_wnd, "Developed by Dr. Arno Puder for educational\n");
   output_string(&shell_wnd, "and instructional purposes for SFSU CSc 720\n");
   output_string(&shell_wnd, "course.\n");
   output_string(&shell_wnd, "Current shell implementation by Moses Lee\n");

}

//Helper function for processing flags and options
void process_flags(const char * flags, 
                   char * flag_options, 
                   char * flag_values, 
                   int flag_char_count)
{
   //Clear the buffer
   clear_buf(flag_options, 8);
   clear_buf(flag_values, 8);

   //Parse through the flag options for the flag and its value if it has one
   int i, collect_value = FALSE, option_cnt = 0, value_cnt = 0;
   for(i = 0; i < flag_char_count; i++)
   {
      //If there is a space detected, then start collecting arguments for the flags
      if(flags[i] == ' ' || flags[i] == (char) 0)
      {
         collect_value = TRUE;
      }
      //Collect values for the specified flags
      else if(flags[i] != ' ' && collect_value == TRUE)
      {
         flag_values[value_cnt++] = flags[i];
      }
      //Collect characters of the flag
      else
      {
         flag_options[option_cnt++] = flags[i];
      }
   }

#if DEBUG
   kprintf("option count %d flag_char_count: %d\n", option_cnt, flag_char_count);
   print_current_cmd(flag_options, option_cnt);
#endif

   //Null terminating at the end
   flag_values[value_cnt] = '\0';
   flag_options[option_cnt] = '\0';
}

//Prepares Train commands
void relay_train_commands(const char * flags, int flag_char_count)
{

   static char * flag_list[] = {
                                 "h",
                                 "m",
                                 "r",
                                 "t",
                                 "init"
                               };

   COM_Message msg;
   char buffer[12];
   msg.input_buffer = buffer; msg.len_input_buffer = 0;
   char stripped_flags[9];
   char flag_value[9];
   process_flags(flags, stripped_flags, flag_value, flag_char_count); 


   //Begin Train commands
   if(0 == k_strcmp(stripped_flags, flag_list[0]))
   {
      output_string(&shell_wnd, "----------------- Welcome to Train -----------------\n");  
      output_string(&shell_wnd, "-h          ------- For available options.\n");  
      output_string(&shell_wnd, "-m [s]      ------- Move train at Speed [s]\n");  
      output_string(&shell_wnd, "-r          ------- Reverse direction of train\n");  
      output_string(&shell_wnd, "-t [n][r/g] ------- Change track [n] to color[r/g]. i.e. 5G\n");  
      output_string(&shell_wnd, "-init       ------- Start train process\n");
   }
   else if(0 == k_strcmp(stripped_flags, flag_list[1]))
   {
      //Move train at specified speed
      if(5 < k_atoi(flag_value) || k_atoi(flag_value) < 0)
         output_string(&shell_wnd, "Speed must be a value between 0 and 5\n");
      else
      {
         msg.output_buffer = "L20S5\015";
         msg.output_buffer[4] = flag_value[0];
         send(com_port, &msg);
      }
   }
   else if(0 == k_strcmp(stripped_flags, flag_list[2]))
   {
      //Reverse the direction of train
      msg.output_buffer = "L20D\015\0";
      send(com_port, &msg);
   }
   else if(0 == k_strcmp(stripped_flags, flag_list[3]))
   {
      //Change track
      char track[] = "MNC\015";

      //Check to see if track number is within range
      if(9 < k_atoi(&flag_value[0]) || k_atoi(&flag_value[0]) < 1)
      {
         output_string(&shell_wnd, "Track number must be between 1 and 9\n");
         return;
      }
      
      //Check to see if the route change is valid
      if((flag_value[1] != 'g' && flag_value[1] != 'G') &&
         (flag_value[1] != 'r' && flag_value[1] != 'R'))
      {
         output_string(&shell_wnd, "Not a valid track route\n");
         return;
      }
      
      //Set the track number
      track[1] = flag_value[0];

      //To accept both lower case and upper case arguments
      track[2] = (flag_value[1] < 91 ) ? flag_value[1] : flag_value[1] - 32;

      //Set the message buffer and send
      msg.output_buffer = track;
      send(com_port, &msg);
   }
   else if(0 == k_strcmp(stripped_flags, flag_list[4]))
   {
      //Initialize the train process
      init_train(&train_wnd);
   }
   else
   {
      output_string(&shell_wnd, "Invalid train options. Pass -h for usage\n");
   }
}

//Start the pacman game with number of ghost user specifies
void start_pacman_game(const char * flags, int flag_char_count)
{
   static char * option_list[] = {
                                  "h",
                                  "g",
                                  "d"
                                 };
   char stripped_flags[9];
   char flag_value[9];

   process_flags(flags, stripped_flags, flag_value, flag_char_count); 


   if(0 == k_strcmp(stripped_flags, option_list[0]))
   {
      //Print out help for pacman
      output_string(&shell_wnd, "---------------- Welcome to Pacman ----------------\n");  
      output_string(&shell_wnd, "-h     ------ For available options.\n");  
      output_string(&shell_wnd, "-g [n] ------ Init pacman with [n] ghosts. Max 4.\n");  
      output_string(&shell_wnd, "-d     ------ Init pacman with default 4 ghosts.\n");  
   }
   else if(0 == k_strcmp(stripped_flags, option_list[1]))
   {
      //Initialize pacman with user specified number of ghost
      //Max 4 ghosts
      int num_ghosts = k_atoi(flag_value);
      if(num_ghosts > 4)
         output_string(&shell_wnd, "Number of ghosts exceeded\n");
      else
         init_pacman(&pac_wnd, num_ghosts);
   }
   else if(0 == k_strcmp(stripped_flags, option_list[2]))
   {
      //Default start pacman with 4 ghosts
      init_pacman(&pac_wnd, 4);
   }
   else
   {
      //If the passed wrong flags
      output_string(&shell_wnd, "Invalid pacman options. Pass -h for usage\n");
   }
}


//Strips of all unnecessary characters and executes commands based on list of commands
void execute_command(char * cmd, int char_count)
{
   static char * function_list[] = { "help",
                                     "about",
                                     "clear",
                                     "ps",
                                     "pacman",
                                     "train"
                                   };

   //Prepare buffers for command and possible flags
   int cmd_count = 0;
   char stripped_cmd[MAX_BUF_COM-1];
   int flag_count = 0;
   char flags[MAX_FLAG_CHAR];

   //Clear out buffers
   clear_buf(stripped_cmd, MAX_BUF_COM-1);
   clear_buf(flags, 16);

   //Strip all white space and collect flags if present
   int i;
   int flags_found = FALSE;
   for(i = 0; i < char_count; i++)
   {
      //Space ignore
      if(cmd[i] == ' ' && flags_found != TRUE) continue;
      
      //If a '-' is found, beginning of flag
      if(cmd[i] == '-')
      {
         flags_found = TRUE;
      }
      else
      {
         //If flags are not set then keep collecting character for commands
         if(flags_found != TRUE)
         {
            if(cmd_count >= MAX_BUF_COM-1) break;
            stripped_cmd[cmd_count++] = cmd[i];
         }
         //If flags are set then collect characters for the optional flags
         else
         {
            if(flag_count >= MAX_FLAG_CHAR) break;
            flags[flag_count++] = cmd[i];
         }
      }
   }

   stripped_cmd[cmd_count+1] = '\0';

#if DEBUG
   kprintf("char_count: %d Count: %d\n", char_count, cmd_count);
   print_current_cmd(stripped_cmd, cmd_count);
   print_current_cmd(flags, flag_count);
#endif

   //Determine which command to run
   if(0 == k_strcmp(function_list[0], stripped_cmd))
   {
      //Prints menu with available commands
      print_help_menu(flags);
   } 
   else if(0 == k_strcmp(stripped_cmd, function_list[1]))
   {
      //Prints credits
      print_credits(flags);
   }
   else if(0 == k_strcmp(stripped_cmd, function_list[2]))
   {
      //Clears the window. If flag is passed, it is an invalid option
      if(flags_found != FALSE)
         output_string(&shell_wnd, "Invalid option\n");
      else
         clear_window(&shell_wnd);
   }
   else if(0 == k_strcmp(stripped_cmd, function_list[3]))
   {
      //Prints all current processes. If flag is passed, it is an invalid option
      if(flags_found != FALSE)
         output_string(&shell_wnd, "Invalid option\n");
      else
         print_all_processes(&shell_wnd);
   }
   else if(0 == k_strcmp(stripped_cmd, function_list[4]))
   {
      //Start the pacman game
      start_pacman_game(flags, flag_count);
   }
   else if(0 == k_strcmp(stripped_cmd, function_list[5]))
   {
      //Start train programs
      relay_train_commands(flags, flag_count);
   }
   else
   {
      //Command not found
      output_string(&shell_wnd, "Command not found\n");
   }
}

//Main Shell Process
void shell_process(PROCESS self, PARAM param)
{
   Keyb_Message msg;
   clear_window(kernel_window);

   //Buffer to hold current command
   char command[MAX_BUF_COM];
   clear_buf(command, MAX_BUF_COM);
   move_cursor(&shell_wnd, 0, 0);
   show_cursor(&shell_wnd);

   int char_count = 0;
   while(1)
   {
      //Read command from keyboard
      send(keyb_port, &msg);

      char curr_char = *msg.key_buffer;

      switch(curr_char){
         case ENTER_COM:
            output_char(&shell_wnd, '\n');

            //If user did not enter any characters just enter new line
            if(char_count == 0)
               output_string(&shell_wnd, "\n");
            //If user did enter character see if it is a valid command
            else
               execute_command(command, char_count);
            
            clear_buf(command, MAX_BUF_COM);
            char_count = 0;
            break;
           
         //Handle backspace characters
         case BACKSPACE_COM:
            if(char_count <= 0) break;

            modify_buffer_at_index((char) 0, command, char_count);
            output_char(&shell_wnd, curr_char);
            char_count--;
            break;
            
         //For all other characters add it to the command buffer
         default:
            if(char_count >= MAX_BUF_COM) break;
            modify_buffer_at_index(curr_char, command, char_count);
            output_char(&shell_wnd, curr_char);
            char_count++;
            break;
      }
   }
}

void init_shell()
{
   create_process(shell_process,
                  3,
                  0,
                  "Shell Process");
   resign();
}
