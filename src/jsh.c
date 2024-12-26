#include "../include/jsh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

// read line function
#define JSH_RL_BUFFSIZE 1024
char *jsh_rl(void) {
  int buffsize = JSH_RL_BUFFSIZE;
  int pos = 0;
  char *buf = malloc(sizeof(char)*buffsize);

  // store char as int to check for EOF, char is a numeric type 
  // so it gets implicitly casted 
  int character;


  for(;;) {
    character = getchar(); 
    // No multiline commands atm, will implement for Gauss Jordan (jaclar)
    if (character == EOF || character == '\n') {
      buf[pos] = '\0';
      return buf;
    } else buf[pos] = character;
    pos++;

    if (pos >= buffsize) {
      buffsize += JSH_RL_BUFFSIZE;
      buf = realloc(buf, buffsize);
      // Bad alloc, just get a good pc
      if (!buf) {
        // TO:DO implement 2> redirection for bad alloc logs
        fprintf(stderr, "jsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}


// tokenizer function

#define JSH_TOK_BUFFSIZE 64
#define JSH_TOK_DELIM " \t\r\n\a"
char **jsh_tokenize(char *l) {
  int buffsize = JSH_TOK_BUFFSIZE, pos = 0;
  char **tokens = malloc(buffsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "jsh: allocation error\n");
    exit(EXIT_FAILURE);
  }
  
  // opt: use strsep
  token = strtok(l, JSH_TOK_DELIM);
  while (token!=NULL) {
    tokens[pos++] = token;

    if (pos >= buffsize) {
      buffsize += JSH_TOK_BUFFSIZE;
      tokens = realloc(tokens, buffsize * sizeof(char*));
      
      if (!tokens) {
        fprintf(stderr, "jsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, JSH_TOK_DELIM);
  }
  tokens[pos] = NULL;
  return tokens;
}


// to execute stuff:
// first, an existing process forks (fork() syscall) itself into two separate ones.
// Then, child uses exec() syscall to replace itself with a brand new program!
// Parent continues doing other things, even keeping tabs with wait()

int jsh_fetch(char **args) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  // execvp should never return something
  if (pid == 0){
    if (execvp(args[0], args) == -1) {
      perror("jsh");
    } 
  } else if (pid < 0) {
    perror("jsh");
  } else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

// BUILTIN TIME


char *commands[] = {
  "cd",
  "help",
  "exit"
};

int (*command_fn[]) (char**) = {
  &jsh_cd,
  &jsh_help,
  &jsh_exit
};

int jsh_num_commands(void) {
  return sizeof(commands) / sizeof(char*);
}

int jsh_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "jsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("jsh");
    }
  }
  return 1;
}


int jsh_help(char **args) {
  printf("Juan Torres's jsh\n");
  printf("Type program names and arguments, hit enter. \n");
  printf("The following are builtins: \n");

  for (int i = 0; i < jsh_num_commands(); i++)
    printf("  %s\n", commands[i]);

  return 1;
}

// this signals REPL to terminate
int jsh_exit(char **args) {
  return 0;
}

int jsh_exec(char **args) {
  if (args[0] == NULL)
    return 1;
  
  for (int i = 0; i < jsh_num_commands(); i++) {
    if (strcmp(args[0], commands[i]) == 0) {
      return (*command_fn[i])(args);
    }
  }
  
  return jsh_fetch(args);
}

// loop function
void jsh_runtime(void) {
  char *l;
  char **args;
  int status_flag;

  do {
    printf("ƒ> ");
    l = jsh_rl();
    args = jsh_tokenize(l);
    status_flag = jsh_exec(args);
    
    // free heap-allocated data
    free(l);
    free(args);
  } while(status_flag);
}


