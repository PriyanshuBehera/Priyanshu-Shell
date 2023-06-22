#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include<dirent.h>

#define HISTORY_MAX_SIZE 10 
#define MAX_COMMAND_LENGTH 100

 char history[HISTORY_MAX_SIZE][MAX_COMMAND_LENGTH];
 int history_count = 0;


#define builtins_num 9
int cnt;
char *read_line(void)
{
  char *line = NULL;
  ssize_t bufsize = 0; 

  if (getline(&line, &bufsize, stdin) == -1){
    if (feof(stdin)) {
      exit(EXIT_SUCCESS); 
    } else  {
      perror("readline");
      exit(EXIT_FAILURE);
    }
  }

  return line;
}
#define DFLT "\033[0m"

#define arg_size 64
#define delimeters " \t\r\n\a"
char **parser(char *command)
{
  int bufsize = 64, position = 0;
  char **args = malloc(bufsize * sizeof(char*));
  char *arg;

  if (!args) {
    fprintf(stderr, "psh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  arg = strtok(command, delimeters);
  while (arg != NULL) {
    args[position] = arg;
    cnt++;
    position++;

    if (position >= bufsize) {
      bufsize += arg_size;
      args = realloc(args, bufsize * sizeof(char*));
      if (!args) {
        fprintf(stderr, "psh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    arg = strtok(NULL, delimeters);
  }
  args[position] = NULL;
  return args;
}

int launch(char **args){
  pid_t pid,wpid;
  int status;
  pid = fork();
  if(pid==0){
    if(execvp(args[0],args)==-1){
      perror("psh");
    }
    exit(EXIT_FAILURE);
  }
  else if(pid<0){
    perror("psh");
  }
  else{
    do{
      wpid = waitpid(pid,&status,WUNTRACED);
    }while(!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 1;
}

char *builtin_str[] = {
  "cd",
  "info",
  "exit",
  "echo",
  "pwd",
  "ls",
  "disp_hist",
  "makedir",
  "removedir"
};

int change_dir(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "psh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("psh");
    }
  }
  return 1;
}

int info(char **args)
{
  
  printf("Priyanshu's Shell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (int i = 0; i < builtins_num; i++) {
    printf("  %s\n", builtin_str[i]);
  }

  return 1;
}

int exit_shell(char **args)
{
  return 0;
}

int echo(char** args){
    for(int i=1;i<cnt;i++){
    printf("%s ",args[i]);
    }
    printf("\n");
    return 1;
}

int pwd(){
    char home[1000];
    getcwd(home,1000);
    printf("%s\n",home);
    return 1;
}

int ls(char** args){
  DIR *dir;
  struct dirent *entry;
  if(args[1]==NULL){
    dir = opendir(".");
  }
  else if(args[2]==NULL){
    dir = opendir(args[1]);
  }
  else{
    printf("Usage: ls [directory]\n");
        return 1;
  }
  if (dir == NULL) {
        perror("opendir");
        return 1;
    }
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }
    return 1;
}


void add_to_history(char** args){
  char command[MAX_COMMAND_LENGTH] = "";
  for (int i = 0; args[i] != NULL; i++) {
        strcat(command, args[i]);
        strcat(command, " ");
    }

    command[strcspn(command, "\n")] = '\0';

    if (history_count < HISTORY_MAX_SIZE) {
        strcpy(history[history_count], command);
        history_count++;
   } 

   else {
        for (unsigned index = 1; index < HISTORY_MAX_SIZE; index++) {
            strcpy(history[index-1], history[index]);
        }
        strcpy(history[HISTORY_MAX_SIZE - 1], command);
    }
}

int disp_hist(){
  if(history_count==0){
    printf("%s\n","No History to print");
    return 1;
  }
  for (int i = 0; i < history_count; i++) {
        printf("%s\n", history[i]);
    }
    return 1;
}

int makedir(char** args){
  if(args[1]==NULL){
    printf("%s\n","Expected command: makedir dirname");
    return 1;
  }
  int check;
  check = mkdir(args[1],0777);
   if (!check){
        printf("Directory created\n");
    return 1;
   }
    else {
        printf("Unable to create directory\n");
        return 1;
    }
  
    
}

int removedir(char** args){
  if(args[1]==NULL){
    printf("%s\n","Expected command: removedir dirname");
    return 1;
  }
  int check;
  check = rmdir(args[1]);
   if (!check){
        printf("Given directory removed successfully\n");
    return 1;
   }
    else {
        printf("Unable to remove directory\n");
        return 1;
    }
}

int execute(char** args){
    int status;
    if(args[0]==NULL){
        return 1;
    }
    for(int i=0;i<builtins_num;i++){
        if(strcmp(args[0],builtin_str[i])==0){
            if(i==0){
                add_to_history(args);
                return change_dir(args);
            }
            else if(i==1){
                add_to_history(args);
                return info(args);
            }
            else if(i==2){
                add_to_history(args);
                return exit_shell(args);
            }
            else if(i==3){
              add_to_history(args);
              return echo(args);
            }
            else if(i==4){
              add_to_history(args);
              return pwd();
            }
            else if(i==5){
              add_to_history(args);
              return ls(args);
            }
            else if(i==6){
              add_to_history(args);
              return disp_hist();
            }
            else if(i==7){
              add_to_history(args);
              return makedir(args);
            }
            else{
              add_to_history(args);
              return removedir(args);
            }
        }
    }
    return launch(args);
}



    
void processloop(){
    char* command;
    char **args;
    int status = 1;
    do{
        printf(">>>> ");
        cnt = 0;
        command = read_line();
        args = parser(command);
        
        status = execute(args);

        free(command);
        free(args);

    }while(status);
}
int main(int argc,char **argv){
    
    processloop();
    return 0;
}
