#ifndef JSH_H
#define JSH_H

// util
void jsh_runtime(void);
char *jsh_rl(void);
char **jsh_tokenize(char*);
int jsh_fetch(char**);

//  builtins
int jsh_cd(char**);
int jsh_help(char**);
int jsh_exit(char**);
int jsh_num_commands(void);

#endif
