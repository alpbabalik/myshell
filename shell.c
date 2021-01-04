#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

const int max_num_args = 20;
const int max_cmd_len = 1024;
int first_call = 1;
// I - initial state
// F - foreground state
// B - backround state
// S - stopped
enum state{I,F,B,S};
struct process {
  pid_t pid;
  enum state process_state;
  int job_id;
  char cmdline[max_cmd_len];
};
struct process *jobs;

void print_error(char *msg){
  printf("%s\n", msg);
  exit(1);
}
void malloc_error_check(void *ptr){
  if(ptr == NULL){
    printf("Memory allocation error!\n");
    exit(1);
  }

}

char* give_prompt(){
  char prompt[100] = "";
  char *lgn;
  char *command_line;
  char host[256];
  char currDir[1024] = "";
  int hostname = gethostname(host, sizeof(host));
  lgn = (char*)malloc(20*sizeof(char));
  malloc_error_check(lgn);
  command_line = malloc(max_cmd_len);
  malloc_error_check(command_line);
  getlogin_r(lgn, 20);
  if(first_call){
    // To clean the screen
    const char* CLEAR_SCREEN_ANSI = " \e[1;1H\e[2J";
    write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
    first_call = 0;
  }
  sprintf(prompt,"(base) %s@%s %s %% ",lgn, host, getcwd(currDir, sizeof(currDir))); 
  
  command_line= readline(prompt);
  return command_line;
}


char** parse_command_line(char *command_line){
  char** arg_list = malloc(sizeof(char*)*max_num_args);
  malloc_error_check(arg_list);
  const char delimeter[] = " \n";
  char local_command_line[max_cmd_len];
  char* ptr = local_command_line;
  char* token;
  int count = 0;
  strcpy(ptr, command_line);
  while(*ptr == ' '){
    ptr++;
  }
  token = strtok(ptr, delimeter);
  if(token == NULL)
    print_error("Unable to tokenize user input");
  while(token != NULL){
    if(count >= max_num_args)
      print_error("Exceeded max number of arguments");
    arg_list[count] = strdup(token);
    token = strtok(NULL, delimeter);
    count++;
  }
  return arg_list;
}

int main(int argc, char* argv[]){
  int counter;
  int maxjobs = 10;
  char* command_line;
  int process_id;
  char **arg_list;
  if(argc >= 2){
    for(counter = 1; counter<argc; counter++){
      if(strcmp(argv[counter], "MAXJOBS") == 0){
	      if(atoi(argv[counter+1]) <= 0)
	        print_error("Maxjobs argument value must be greater than 0");
        else
	        maxjobs = atoi(argv[counter+1]);
      }
      else{
	      if(strcmp(argv[counter], "-h") == 0)
	        print_error("----HELP----\n-h -> manual page\nMAXJOBS -> place this arguments to modify deafult max jobs value\n");
      }
    }
   }
  jobs = malloc(maxjobs * sizeof(struct process));
  malloc_error_check(jobs);
  for(int i = 0; i<maxjobs; i++){
    jobs[i].pid = 0;
    jobs[i].process_state = I;
    jobs[i].job_id = 0;
    jobs[i].cmdline[0] = ' ';
  }
  
  while(1){
    command_line = give_prompt();
    add_history(command_line);
    if(command_line == NULL)
      print_error("Unable to read from stdin. Please check your input and try again!");
    strcat(command_line,"\n");
    arg_list = parse_command_line(command_line);
    free(command_line);
    process_id = fork();
    if(process_id < 0)
      print_error("Failed to create child process");
    else if(process_id == 0){
      if(execvp(arg_list[0], arg_list) < 0)
	      print_error("Failed to execute given command");
      free(arg_list);
    }
    else {
      wait(NULL);
    }
  }
  return 0;
}

