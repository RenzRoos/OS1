#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h> 

#define len 1000

int tokenizer(char * token[len], char input[], int * pipeline){

    int i = 0;
    char * temp;
    temp = strtok(input, " ");

    while(temp != NULL){ 
        token[i] = temp;

        while(token[i][0] == ' '){
            token[i] = token[i] + 1;
        }
        while(token[i][strlen(token[i]) - 1] == ' '){
            token[i][strlen(token[i]) - 1] = '\0';
        }
        if(token[i][0] == '|'){
            *pipeline = 1;
        }
        temp = strtok(NULL, " ");
        i++;
    }

    return i;
}

void cd(char * token[], int * i){
    (*i)++;    
    char * temp = token[*i];
    while(temp[0] == ' '){
        temp = temp + 1;
    }
    while(temp[strlen(temp) - 1] == ' '){
        temp[strlen(temp) - 1] = '\0';
    }
    chdir(temp);
} 

void path_arguments_flags_setter(char path[len], char flags[len], char arguments[len], char * token[], int token_size, int * i){
    if(token[*i][0] != '.' && token[*i][1] != '/'){ 
        strncat(path, token[*i], len);
        (*i)++;
    }

    int multi_arg = 0;
    
    while(*i < token_size && token[*i][0] == '-'){
        if(multi_arg == 1){
            strncat(flags, " ", strlen(flags) + 1);
        }
        strncat(flags, token[*i], strlen(token[*i]));
        (*i)++;
        multi_arg = 1;
    }

    multi_arg = 0;

    while(*i < token_size && token[*i][0] != '|'){
        if(multi_arg == 1){
            strncat(arguments, " ", strlen(arguments) + 1);
        }
        strncat(arguments, token[*i], len);
        (*i)++;

        multi_arg = 1;
    }
}

void args_setter(char *args[4], char path[len], char flags[len], char arguments[len]){
    if(strncmp(path,"/bin/", strlen(path)) == 0){
        memset(path, 0, strlen(path));
        strncat(path, arguments, strlen(arguments));
        args[0] = NULL;
        args[1] = NULL;
        args[2] = NULL;
    }
    else if(flags[0] == '\0' && arguments[0] == '\0'){
        args[1] = NULL;
        args[2] = NULL;
    }
    else if(flags[0] == '\0'){
        args[1] = arguments;
        args[2] = NULL;
    }
    else if(arguments[0] == '\0'){
        args[2] = NULL;
    }
}

int main() {

    while(1){

        pid_t pid;
        pid_t pid2;

        char input[len];
        char * token[len];
        int token_size;
        int i = 0;
        
        int pipeline = 0;
        int pipefd[2];

        char cwd[len];
        getcwd(cwd, sizeof(cwd));
        printf("%s >>> ", cwd);
        
        fgets(input, len, stdin);

        input[strlen(input) - 1] = '\0';
        token_size = tokenizer(token, input, &pipeline);
        
        pid = fork();

        if(pid < 0 || pid2 < 0){
            fprintf(stderr, "Fork failed\n");
            return 1;
        }
        else if(strncmp(token[i], "exit", 4) == 0){
            return 0;
        }
        else if( (pid == 0 || pid2 == 0) && strncmp(token[i], "cd", 2) == 0){
            cd(token, &i);
        }
        else if(pid == 0 || pid2 == 0){
            char path[len] = "/bin/";
            char arguments[len] = "\0";
            char flags[len] = "\0";

            path_arguments_flags_setter(path, flags, arguments, token, token_size, &i);
            
            char *args[4] = {path, flags, arguments, NULL};
        
            args_setter(args, path, flags, arguments);

            if(token[i][0] == '|'){
                pid2 = fork();
            }

            execv(path, args);
        }
        else{
            wait(NULL);
            printf("Child complete\n");
        }            
        
    }
    return 0;
}