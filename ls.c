#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h> 

#define len 1000

char * tokenizer(char * token, char input[]){

    token = strtok(input, "|");

    return token;
}

int main() {
    pid_t pid;
    char buf[len];

    char input[len];
    char * token;
       

    while(1){
        printf("Command?\n");
        fgets(input, len, stdin);
        input[strlen(input) - 1] = '\0';

        token = tokenizer(token, input);
                    

        while(token != NULL) {
            pid = fork();
            while(token[0] == ' '){
                token = token + 1;
            }

            if(pid < 0){
                fprintf(stderr, "Fork failed");
                return 1;
            }
            else if(pid == 0 && strncmp(token, "ls", 2) == 0){
                char *args[] = {"/bin/ls", ".", NULL};
                execv("/bin/ls", args);
            }
            else if(pid == 0 && strncmp(token, "exit", 4) == 0){
                exit(0);
            }
            else if(pid == 0 && strncmp(token, "cd", 2) == 0){
                
                char * temp = token + 3;
                while(temp[0] == ' '){
                    temp = temp + 1;
                }
                while(temp[strlen(temp) - 1] == ' '){
                    temp[strlen(temp) - 1] = '\0';
                }
                printf("temp:%s6\n", temp);
                chdir(temp);
            }
            else{
                wait(NULL);
                printf("Child complete\n");
            }
            
            token = strtok(NULL, "|");
        }
    }
    return 0;
}