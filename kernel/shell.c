#include <kernel.h>

#define ENTER_COM     0x0D
#define BACKSPACE_COM 0x08
#define MAX_BUF_COM   60
#define MAX_FLAG_CHAR 16
#define DEBUG 0

WINDOW shell_wnd  = {0, 9, 61, 16, 0, 0, 0xDC};
WINDOW pac_wnd = {61, 9, 19, 16, 0, 0, 0xDC};

//Helper debug function
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
   {
      com[i] = (char) 0;
   }
}

//Modifies the character buffer as needed
void modify_buffer_at_index(char c, char * buf, int index)
{
   //If index is greater or below the buffer count, just return
   if(index > MAX_BUF_COM - 1 || index < 0) return;

   buf[index] = c;
}

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
   clear_buf(flag_options, 8);
   clear_buf(flag_values, 8);

   int i, collect_value = FALSE, option_cnt = 0, value_cnt = 0;
   for(i = 0; i < flag_char_count; i++)
   {
      if(flags[i] == ' ' || flags[i] == (char) 0)
      {
         collect_value = TRUE;
      }
      else if(flags[i] != ' ' && collect_value == TRUE)
      {
         flag_values[value_cnt++] = flags[i];
      }
      else
      {
         flag_options[option_cnt++] = flags[i];
      }
   }

#if DEBUG
   kprintf("option count %d flag_char_count: %d\n", option_cnt, flag_char_count);
#endif

   flag_values[value_cnt] = '\0';
   flag_options[option_cnt] = '\0';
}

//Prepares Train commands
void relay_train_commands(const char * flags, int flag_char_count)
{

   static char * flag_list[] = {
                                 "h",
                                 "mf",
                                 "mb",
                                 "stop",
                                 "init",
                                 "sim"
                               };

   char stripped_flags[9];
   char flag_value[9];

   process_flags(flags, stripped_flags, flag_value, flag_char_count); 

#if DEBUG
   print_current_cmd(stripped_flags, 8);
   kprintf("%s --- %s\n", stripped_flags, flag_list[0]);
#endif

   //Begin Train commands
   if(0 == k_strcmp(stripped_flags, flag_list[0]))
   {
      output_string(&shell_wnd, "----------------- Welcome to Train -----------------\n");  
      output_string(&shell_wnd, "-h      ------- For available options.\n");  
      output_string(&shell_wnd, "-mf [s] ------- Move train forward at Speed [s]\n");  
      output_string(&shell_wnd, "-mb [s] ------- Move train backward at Speed [s]\n");  
      output_string(&shell_wnd, "-stop   ------- Stop train\n");  
      output_string(&shell_wnd, "-init   ------- Start train process\n");
      output_string(&shell_wnd, "-sim    ------- Start train simulation\n");
   }
   else if(0 == k_strcmp(stripped_flags, flag_list[1]))
   {
   }
   else if(0 == k_strcmp(stripped_flags, flag_list[2]))
   {
   }
   else if(0 == k_strcmp(stripped_flags, flag_list[3]))
   {
   }
   else if(0 == k_strcmp(stripped_flags, flag_list[4]))
   {
   }
   else if(0 == k_strcmp(stripped_flags, flag_list[5]))
   {
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
      output_string(&shell_wnd, "---------------- Welcome to Pacman ----------------\n");  
      output_string(&shell_wnd, "-h     ------ For available options.\n");  
      output_string(&shell_wnd, "-g [n] ------ Init pacman with [n] ghosts. Max 4.\n");  
      output_string(&shell_wnd, "-d     ------ Init pacman with default 4 ghosts.\n");  
   }
   else if(0 == k_strcmp(stripped_flags, option_list[1]))
   {
      int num_ghosts = k_atoi(flag_value);
      if(num_ghosts > 4)
      {
         output_string(&shell_wnd, "Number of ghosts exceeded\n");
      }
      else
      {
         init_pacman(&pac_wnd, num_ghosts);
      }
   }
   else if(0 == k_strcmp(stripped_flags, option_list[2]))
   {
      init_pacman(&pac_wnd, 4);
   }
   else
   {
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
      if(cmd[i] == ' ') continue;
      
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
            stripped_cmd[cmd_count++] = cmd[i];
            if(cmd_count >= MAX_BUF_COM-1) break;
         }
         //If flags are set then collect characters for the optional flags
         else
         {
            flags[flag_count++] = cmd[i];
            if(flag_count++ >= MAX_FLAG_CHAR) break;
         }
      }
   }

   stripped_cmd[cmd_count+1] = '\0';

#if DEBUG
   kprintf("char_count: %d Count: %d\n", char_count, cmd_count);
   print_current_cmd(stripped_cmd, cmd_count);
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

#if DEBUG
            print_current_cmd(command, char_count);
#endif
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
                  7,
                  0,
                  "Shell Process");
   resign();
}
