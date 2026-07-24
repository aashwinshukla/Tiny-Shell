#include<stdio.h>
#include<string.h>
#include<stdbool.h>

char command[10];
char explain[5];
bool enter_command = true;

int check(const char *command);
void help();
void defination();

int main(){
    printf("\n--------------Tiny Shell--------------\n");
    printf("\n             WELCOME USER             \n");

    while(enter_command){
            printf("Enter your command:\n");
            printf("[OR ./help]\n");
            scanf(" %s", command);

            if(check(command)){
                if (strcmp(command, "./help") == 0) {
                    help();
                    scanf(" %s",explain);
                        if(strcmp(explain, "./expl") == 0){
                            defination();
                        }
                    enter_command = true;
                } 
                else if (strcmp(command, "") == 0) {
                
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

void help(){
    printf("\n--------------Command List--------------\n");
    printf("1. ./help\n");
    printf("2. pwd\n");
    printf("3. ls\n");
    printf("4. date\n");
    printf("5. whoami\n");
    printf("6. mkdir\n");
    printf("7. echo\n");
    printf("8. exit\n");
    printf("9. clear\n");
    printf("10. cd\n");
    printf("11. history\n");
    printf("\nto know more type ./expl [OR type anything]\n");

}

void defination(){
    printf("\n--------------Command Brief--------------\n");
    printf("\n1. ./help - Display the list of available commands\n");
    printf("2. pwd - Print Working Directory (shows current directory path)\n");
    printf("3. ls - List files and directories in the current directory\n");
    printf("4. date - Display the current date and time\n");
    printf("5. whoami - Display the current user name\n");
    printf("6. mkdir - Make Directory (create a new directory)\n");
    printf("7. echo - Display a line of text or string\n");
    printf("8. exit - Exit the shell program\n");
    printf("9. clear - Clear the terminal screen\n");
    printf("10. cd - Change Directory (navigate to a different directory)\n");
    printf("11. history - Display command history\n");
    printf("\n-----------------------------------------\n\n");
}
