#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h> 

#define len 1000

int main(int argc, char ** argv) {
    pid_t pid;
    char buf[len];

    char binaryPath[len] = "/bin/";
    char token[len];

    fgets(token, sizeof(token), stdin);

    while (1){
        pid = fork();
        if(pid < 0){
            fprintf(stderr, "Fork failed");
            return 1;
        }
        else if(pid == 0 && strcmp(token, "ls") == 0){
            char *args[] = {binaryPath, ".", NULL};
            strcat(binaryPath, token);
            execv(binaryPath, args);
            return 0;
        }
        else if(pid == 0 && strcmp(token, "exit") == 0){
            return 0;
        }
        else if(pid == 0 && strcmp(token, "cd") == 0){

            fgets(token, sizeof(token), stdin);

            printf("%s\n", getcwd(buf, len));
            
            // using the command
            chdir(token);
        
            // printing current working directory
            printf("%s\n", getcwd(buf, len));
        }
        else{
            wait(NULL);
            printf("Child complete\n");
        }
        fgets(token, sizeof(token), stdin);
        char binaryPath[len] = "/bin/";
    }
}