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
   output_string(&shell_wnd, "pacman  ------- Runs pacman simulation\n");
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

//Prepares Train commands
void relay_train_commands(const char * flags, int flag_char_count)
{

   output_string(&shell_wnd, "Train filler\n");
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
         flag_count++;
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
      print_credits(flags);
   }
   else if(0 == k_strcmp(stripped_cmd, function_list[2]))
   {
      if(flag_count != 0)
         output_string(&shell_wnd, "Invalid option\n");
      else
         clear_window(&shell_wnd);
   }
   else if(0 == k_strcmp(stripped_cmd, function_list[3]))
   {
      if(flag_count != 0)
         output_string(&shell_wnd, "Invalid option\n");
      else
         print_all_processes(&shell_wnd);
   }
   else if(0 == k_strcmp(stripped_cmd, function_list[4]))
   {
      if(flag_count != 0)
         output_string(&shell_wnd, "Invalid option\n");
      else
         init_pacman(&pac_wnd, 2);
   }
   else if(0 == k_strcmp(stripped_cmd, function_list[5]))
   {
      relay_train_commands(flags, flag_count);
   }
   else
   {
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
