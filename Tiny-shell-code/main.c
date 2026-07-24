#include<stdio.h>
#include<string.h>
#include<stdbool.h>

char command[10];
bool enter_command = true;

int check();

int main(){
    printf("\n--------------Tiny Shell--------------\n");
    printf("\n             WELCOME USER             \n");

    while(enter_command){
            printf("Enter your command:\n");
            printf("[OR /.help]\n");
            scanf(" %s", command);

            check(command);

    }

    
 

    
    return 0;
}

int check(command){
    if(command == '/.help' ||
    command == 'pwd'||
    command == 'date' ||
    command == 'ls' ||
    command == 'whoami' ||
    command == 'mkdir' ||
    command == 'echo' ||
    command == 'exit' ||
    command == 'clear' ||
    command == 'cd' ||
    command == 'history'){
        enter_command = false;
        break;

    }else{
        printf("\n[ERROR- wrong-input-not-in-the-command-set]\n");
        printf("try-again-OR-use-/.help\n");
        enter_command = true;
    }

    return enter_command;
}