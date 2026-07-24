#include<stdio.h>
#include<string.h>
#include<stdbool.h>

char command[10];
bool enter_command = true;

int check(const char *command);

int main(){
    printf("\n--------------Tiny Shell--------------\n");
    printf("\n             WELCOME USER             \n");

    while(enter_command){
            printf("Enter your command:\n");
            printf("[OR ./help]\n");
            scanf(" %s", command);

            if(check(command)){
                if (strcmp(command, "./help") == 0) {
                    printf("");
                } 
                else if (strcmp(command, "exit") == 0) {
                    enter_command = false;
                }
               
            }
    }

    return 0;
}

int check(const char *command){
    if (strcmp(command, "./help") == 0 ||
        strcmp(command, "pwd") == 0 ||
        strcmp(command, "date") == 0 ||
        strcmp(command, "ls") == 0 ||
        strcmp(command, "whoami") == 0 ||
        strcmp(command, "mkdir") == 0 ||
        strcmp(command, "echo") == 0 ||
        strcmp(command, "exit") == 0 ||
        strcmp(command, "clear") == 0 ||
        strcmp(command, "cd") == 0 ||
        strcmp(command, "history") == 0){
            
            return 1;
    } else {
        printf("\n[ERROR- wrong-input-not-in-the-command-set]\n");
        printf("try-again-OR-use-./help\n\n");

        return 0;
    }
}