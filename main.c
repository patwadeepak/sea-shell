#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SEA_RL_BUFSIZE 1024
char *sea_read_line(void) {
    int bufsize = SEA_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if(!buffer) {
        fprintf(stderr, "sea: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1){
        // Read a character
        c = getchar();

        // If we hit EOF, replace it with a null character and return.
        if (c==EOF || c == '\n'){
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        // If we have executed the buffer, reallocate.
        if (position >= bufsize){
            bufsize += SEA_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer){
                fprintf(stderr, "sea: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define SEA_TOK_BUFSIZE 64
#define SEA_TOK_DELIM " \t\r\n\a"
char **sea_split_line(char *line){
    int bufsize = SEA_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char));
    char *token;

    if (!tokens){
        fprintf(stderr, "sea: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, SEA_TOK_DELIM);
    while(token != NULL){
        tokens[position] = token;
        position++;

        if (position >= bufsize){
            bufsize += SEA_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "sea: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, SEA_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int sea_launch(char **args){
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1){
            perror("sea");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror ("sea");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}


int sea_execute(char **args); 
void sea_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = sea_read_line();
        args = sea_split_line(line);
        status = sea_execute(args);

        free(line);
        free(args);
    } while (status);
}

// Function Declarations for builtin shell commands:
int sea_cd(char **args);
int sea_help(char **args);
int sea_exit(char **args);

// List of builtin commands, followed by their corresponding functions.
char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &sea_cd,
    &sea_help,
    &sea_exit
};

int sea_num_builtins(){
    return sizeof(builtin_str)/sizeof(char *);
}

int sea_execute(char **args) {
    int i;
    
    if (args[0] == NULL){
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < sea_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return sea_launch(args);
}
// Builtin function implementation
int sea_cd(char **args){
    if (args[1] == NULL) {
        fprintf(stderr, "sea: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("sea");
        }
    }
    return 1;
}

int sea_help(char **args){
    int i;
    printf("Deepak Kumar's SEA\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (int i=0; i < sea_num_builtins(); i++){
        printf(" %s\n", builtin_str[i]);
    }
    
    printf("Use the man command for indormation on other programs.\n");
    return 1;
}        

int sea_exit(char **args) {
    return 0;
}



int main() {
    // Load conifg filesm if any.

    // Run command loop.
    sea_loop(); 

    //Perform any shutdown/cleanup

    return EXIT_SUCCESS;
}


