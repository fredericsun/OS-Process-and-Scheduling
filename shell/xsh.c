#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
  char* cwd;
  char buff[PATH_MAX + 1];
  char *cmd_line;
  char **cmd;

  //get in the loop to print the prompt
  while(1){
    //get the current directory
    cwd = getcwd(buff, PATH_MAX + 1);
    prompt_loop(cwd);
    cmd_line = read_command_line();
    cmd = parse_command_line(cmd);
  }
  
}

//print the prompt
void print_prompt(char* cwd){
  if(cwd != NULL){
    printf("[%s]\n", cwd);
    printf("0> ");
  }
  else{
    perror("Current directory error.\n")
    exit(1);
  }
}

//read the input
#define MAX_SIZE_COMMAND_LINE 128;
char *read_command_line(){
  char *input;
  char *buffer = (char *)malloc(bufsize
}

//parse the input
char **parse_command_line(char *cmd_line){
  
}
