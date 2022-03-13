#include <sys/types.h>
#include <sys/stat.h>
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
    int multi_arg = 0;
    (*i)++;
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

int env_finder(char * temp[len]){
    struct stat buf;
    char * path;
    path = strtok(getenv("PATH"), ":");
    int i = 0;
    
    while(path != NULL){ 
        temp[i] = path;
        i++;
        path = strtok(NULL, ":");
    }

    return i;
}

void bin_finder(char bin_path[], char * token, char * temp[len], int i){
    struct stat buf;

    char temp2[len] = "\0";

    for(int j = 0; j < i; j++){
        
        strncat(temp2, temp[j], strlen(temp[j])); 
           

        strncat(temp2, "/", strlen(temp2) + 1);

        strncat(temp2, token, strlen(temp2) + strlen(token));

        if(stat(temp2, &buf) == 0){
            i = j;
            break;
        }
        memset(temp2, 0, strlen(temp2));
    }

    strncat(bin_path, temp2, strlen(temp2));

    return;
}

void args_setter(char *args[4], char path[len], char flags[len], char arguments[len]){
    if(args[2][0] == '.' && args[2][1] == '/'){
        memset(path, 0, strlen(path));
        strncat(path, arguments, strlen(arguments));
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

void wrong_input(int * i, int token_size, int * pipeline){
    fprintf(stderr, "Incorrect input\n");
    *i = token_size;
    *pipeline = 0;
    return;
}

int main() {
    char * temp[len];
    int temp_size = env_finder(temp);
    while(1){

        pid_t pid;
        pid_t pid2;

        char input[len];
        char * token[len];
        int token_size;
        int i = 0;
        
        int pipeline = 0;
        int pipefd[2];

        if(pipe(pipefd) == -1){
            fprintf(stderr, "Pipe failed\n");
            return 1;
        }

        char cwd[len];
        getcwd(cwd, sizeof(cwd));
        printf("%s >>> ", cwd);
        
        fgets(input, len, stdin);

        input[strlen(input) - 1] = '\0';
        token_size = tokenizer(token, input, &pipeline);

        pid = fork();

        if(pid < 0 ){
            fprintf(stderr, "Fork failed\n");
            return 1;
        }
        else if(strncmp(token[i], "exit", 4) == 0 && pipeline == 0){
            return 0;
        }
        else if(pid == 0 && (strncmp(token[i], "cd", 2) == 0)){
            cd(token, &i);
        }
        else if(pid == 0){
            char path[len];
            char arguments[len] = "\0";
            char flags[len] = "\0";
            char bin_path[len];
            
            memset(bin_path, 0, strlen(bin_path));
            bin_finder(bin_path, token[i], temp, temp_size);
            if( strlen(bin_path) == 0 ){
                wrong_input(&i, token_size, &pipeline);
            }
            
            memset(path, 0, strlen(path));
            strncpy(path, bin_path, strlen(bin_path));
            path_arguments_flags_setter(path, flags, arguments, token, token_size, &i);
            
            char *args[4] = {path, flags, arguments, NULL};

            args_setter(args, path, flags, arguments);
            if(pipeline == 1){
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            }
            execv(path, args);
        }
        
        if(pipeline == 1){
            i++;
        }

        if(strncmp(token[i], "|", 1) == 0){
            i++;
            pid2 = fork();
            if(pid2 < 0 ){
                fprintf(stderr, "Fork failed\n");
                return 1;
            }
            else if(strncmp(token[i], "exit", 4) == 0){
                return 0;
            }
            else if(pid2 == 0 && (strncmp(token[i], "cd", 2) == 0)){
                cd(token, &i);
            }
            else if(pid2 == 0){
                char path[len];
                char arguments[len] = "\0";
                char flags[len] = "\0";
                char bin_path[len];
                
                memset(bin_path, 0, strlen(bin_path));
                bin_finder(bin_path, token[i], temp, temp_size);
                if( strlen(bin_path) == 0 ){
                    wrong_input(&i, token_size, &pipeline);
                }
                memset(path, 0, strlen(path));
                strncpy(path, bin_path, strlen(bin_path));
                path_arguments_flags_setter(path, flags, arguments, token, token_size, &i);
                char *args[4] = {path, flags, arguments, NULL};

                args_setter(args, path, flags, arguments);

                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);

                execv(path, args);
            }
        }
       
        if(pipeline == 1){
            close(pipefd[0]);
            close(pipefd[1]);
        }
        waitpid(pid, NULL, 0);
        if(pipeline == 1){
            waitpid(pid2, NULL, 0);
        }
    }
    return 0;
}