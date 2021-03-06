#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

char *read_line(void);
char **line_split_args(char *line);
void colsh_loop(void);
int colsh_launch(char **args);
int colsh_execute(char **args);
int colsh_num_builtins();
int colsh_cd(char **args);
int colsh_help(char **args);
int colsh_exit(char **args);

int (*builtin_func[]) (char**) = {
    &colsh_cd,
    &colsh_help,
    &colsh_exit
};

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

// Main loop
void colsh_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf(">> ");
        line = read_line();
        args = line_split_args(line);
        status = colsh_execute(args);
    } while(status);
}

// Laucnh the command called in a child process
int colsh_launch(char **args) {
    pid_t pid;
    int status;

    pid = fork();
    if(pid == 0) {
        if(execvp(args[0], args) == -1) {
            perror("colsh");
        }
        exit(EXIT_FAILURE);
    } else if(pid < 0) {
        perror("colsh");
    } else {
        do {
            waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

// Given the arg, call the builtin funcion or program
int colsh_execute(char **args) {
    if(args[0] == NULL) {
        return 1;
    }

    for(int i = 0; i < colsh_num_builtins(); i++) {
        if(strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return colsh_launch(args);
}

// Read line, handling EOF for files or Ctrl-D
char *read_line(void) {
    char *line = NULL;
    size_t bufsize = 0; 

    if (getline(&line, &bufsize, stdin) == -1){
        if (feof(stdin)) {  
            exit(EXIT_SUCCESS);
    } else {
        perror("readline");
        exit(EXIT_FAILURE);
    }
  }
  return line;
}

// Split line of args into tokens.
#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"
char **line_split_args(char *line) {
    int bufsize = TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if(!tokens) {
        fprintf(stderr, "colsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOK_DELIM);
    while(token != NULL) {
        tokens[position] = token;
        position++;
        
        if(position >= bufsize) {
            bufsize += TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens) {
                fprintf(stderr, "colsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, TOK_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
}

int colsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

// Change directory based on args[1] 
int colsh_cd(char **args) {
    if(args[1] == NULL) {
        fprintf(stderr, "colsh: expected argument to \"cd\"\n");
    } else if(chdir(args[1]) != 0) {
        perror("colsh");
    }

    return 1;
}

// Give help about the shell
int colsh_help(char **args) {
    puts("Type program names and arguments, and hit enter."); 
    puts("The following are built in:");

    for(int i = 0; i < colsh_num_builtins(); i++) {
        printf("\t%s\n", builtin_str[i]);
    }

    return 1;
}

// Exit the shell
int colsh_exit(char **args) {
    return 0;
}
