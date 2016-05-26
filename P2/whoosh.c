#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define BUFFER_SIZE 129

struct stat statbuf;
void shell_loop(void);
char *shell_read_line(void);
char **shell_split_line(char *line);
int shell_execute(char **args);
int shell_launch(char * new_arg, char **args);
char error_message[30] = "An error has occurred\n";
int position = 0;
char **path;
int pathSize =0;

void shell_loop(void){
  char *line;
  path = malloc(sizeof(char*) * 1);
  path[0] = "/bin";
  pathSize = 1;
  char **args;
  int done = 1;
  while(done){
    write(1, "whoosh> ", 8);
    //read the command frm the standard input
    line = shell_read_line();
    if (position > 129){
      write(STDERR_FILENO, error_message, strlen(error_message));
      continue;
    }
    //seperate the command string into a program and arguments
    args = shell_split_line(line);
    //run the parsed command
    done = shell_execute(args);
    //free(line);
    //free(args);
  } 
}

char *shell_read_line(void){
  int done = 1;
  position = 0;
  int bufsize = BUFFER_SIZE;
  char *buffer =malloc(sizeof(char) * BUFFER_SIZE);
  int c;
  if (!buffer){
    write(STDERR_FILENO, error_message, strlen(error_message));
  }
  //read characters
  while (done == 1){
    c = getchar();
    if (c == EOF || c == '\n'){
      buffer [position] = '\0';
      done = 0;
    }
    else{
      buffer[position] = c;
    }
    position ++;
    if (position >= bufsize) {
      bufsize += BUFFER_SIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        write(STDERR_FILENO, error_message, strlen(error_message));
      }
    }
  }
  return buffer;
}

char **shell_split_line(char *line){
  position = 0;
  char **tokens = malloc(sizeof(char*) * BUFFER_SIZE);
  char *token;
  if (!tokens){
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(EXIT_FAILURE);
  }
  //split characters
  token = strtok(line, " ");
  while (token != NULL){
    tokens[position] = token;
    position ++;
    if (position >= BUFFER_SIZE){
      write(STDERR_FILENO, error_message, strlen(error_message));
      exit(EXIT_FAILURE);
    }
    token = strtok(NULL, " ");
  }
  tokens[position] = NULL;
  return tokens;
}


int shell_launch(char * new_arg, char **args){
  pid_t pid, wpid;
  int status;
  pid = fork();
  if (pid == 0){
    int i=0;
    while (args[i]!= NULL){
      //redirection mode
      if (strcmp(args[i], ">") ==0){
        if (args[i+2]==NULL && args[i+1] != NULL){
          char *out = malloc(strlen(args[i+1])+5);
          char *err = malloc(strlen(args[i+1])+5);
          out[0] = '\0';
          err[0] = '\0';
          strcat(out,args[i+1]);
          strcat(out, ".out");
          strcat(err,args[i+1]);
          strcat(err, ".err");
          close(STDOUT_FILENO);
          if (freopen(out, "w+",stdout)== NULL){
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);  
          }
          if (freopen(err,  "w+", stderr)==NULL){
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);  
          }
          free(out);
          free(err);
          args[i] = NULL;
          args[0] = new_arg;
          execv(new_arg, args);
          write(STDERR_FILENO, error_message, strlen(error_message));
          exit(1);
        }
        else{
          write(STDERR_FILENO, error_message, strlen(error_message));
          exit(1);
        }
      }
      i++;
    }
    args[0] = new_arg;
    execv(new_arg, args);
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(1);
  }
  else if(pid<0){
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(1);
  }
  else{
    do{
      wpid = waitpid(pid, &status, WUNTRACED);
    }while(!WIFEXITED(status)&&!WIFSIGNALED(status));
  }
  return 1;
}

int shell_execute(char **args){
  //printf("%d", position);
  //test the given input whether redirection should be turned on or not
  int redirection = 0;
  int i =0;
  for (i =0; i< position; i++){
    if (strcmp(args[i], ">") == 0){
      redirection = 1;
    }
  }
  if (args[0] == NULL){return 1;}
  //command "exit"
  if (strcmp(args[0], "exit") == 0) {
    if (position != 1){
      write(STDERR_FILENO, error_message, strlen(error_message));
      return 1;
    }
    return 0;
  }
  //command "cd"
  if (strcmp(args[0], "cd") == 0){
    //Code for cd
    char* target_dir;
    if (position >2){
      write(STDERR_FILENO, error_message, strlen(error_message));
      return 1;
    }
    //get the destination, whether it is "HOME" or somewhere else
    if (position == 1){
      char * home_dir;
      home_dir = getenv ("HOME");
      if (home_dir == NULL){
        write(STDERR_FILENO, error_message, strlen(error_message));
        return 1;	
      }
      target_dir = home_dir;
    }
    else {
      target_dir = args[1];
    }
    //not return zero, error returned
    int result = chdir(target_dir);
    if (result != 0){
      write(STDERR_FILENO, error_message, strlen(error_message));
      return 1;
    }
    return 1;
  }
  //command "pwd"
  if (strcmp(args[0], "pwd") == 0){
    if (position == 1) {
      //Code for pwd
      char cwd[128];
      if (getcwd(cwd, sizeof(cwd)) != NULL){
        write(1, cwd, strlen(cwd));
	write(1, "\n", 1); 
      }
      else{
        write(STDERR_FILENO, error_message, strlen(error_message));
      }
    }
    else {
      write(STDERR_FILENO, error_message, strlen(error_message));
    }
    return 1;
  }
  //command "path"
  if (strcmp(args[0], "path") == 0){
    pathSize = 0;
    path = malloc(sizeof(char*) * BUFFER_SIZE);
    /*if (!path){
      write(STDERR_FILENO, error_message, strlen(error_message));
      return 1;
    }*/
    int i =1;
    for (i = 1; i< position; i++){
        path[pathSize] = strdup(args[i]);
        pathSize ++;
    }
    return 1;
  } 
  else{
    char * check;
    int i=0; 
    for (i=0; i<pathSize; i++){
      check = strdup(path[i]);
      strcat(check, "/");
      strcat(check,args[0]);
      if (stat(check, &statbuf) == 0){
        return shell_launch(check, args);
      }
    }
    write(STDERR_FILENO, error_message, strlen(error_message));
    return 1; 
  }
}


int main (int argc, char *argv[]){
  if (argc != 1){
    write (STDERR_FILENO, error_message, strlen(error_message));
    exit(1);
  }
  shell_loop();
  return 0;
}

