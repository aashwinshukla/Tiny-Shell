#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include<unistd.h>
#include<time.h>
#include<dirent.h>
#include<stdlib.h>
#include<sys/stat.h>


char command[10];
char explain[5];
bool enter_command = true;

int check(const char *command);
void help();
void defination();
void run_pwd();
void run_date();
void run_ls();
void run_whoami();
void run_mkdir();
void run_echo();

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
                else if (strcmp(command, "pwd") == 0) {
                    run_pwd();
                    enter_command = true;
                }else if(strcmp(command, "date") == 0){
                    run_date();
                    enter_command = true;
                }else if(strcmp(command, "ls") == 0){
                    run_ls();
                    enter_command = true;
                }else if(strcmp(command, "whoami") == 0){
                    run_whoami();
                    enter_command = true;
                }else if(strcmp(command, "mkdir") == 0){
                    run_mkdir();
                    enter_command = true;
                }else if(strcmp(command, "echo") == 0){
                    run_echo();
                    enter_command = true;
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

void run_pwd(){
    char cwd[1024];
                    if (getcwd(cwd, sizeof(cwd)) != NULL) {
                        printf("%s\n\n", cwd);
                    }
}

void run_date(){
    time_t now = time(NULL);
    printf("%s\n", ctime(&now));
}

void run_ls(){
    DIR *dir = opendir(".");
    struct dirent *entry;

    if (dir == NULL){
        perror("ls error");
        return;
    }

    while((entry = readdir(dir)) != NULL){
        if (entry->d_name[0] != '.'){
            printf("%s  ", entry->d_name);
        }
    }
    printf("\n\n");

    closedir(dir);
}

void run_whoami(){
    char *username = getenv("USER");
    if(username == NULL){
        username = getenv("USERNAME");
    }

    if (username != NULL) {
        printf("%s\n\n", username);
    } else {
        printf("Unknown user\n\n");
    }
}

void run_mkdir(){
    char dirname[100];
    printf("Enter directory name: ");
    scanf(" %s", dirname);

#ifdef _WIN32
    if(mkdir(dirname) == 0){
#else
    if(mkdir(dirname, 0777) == 0){
#endif
        printf("Directory '%s' created successfully.\n\n", dirname);
    }else{
        perror("mkdir error");
    }
}
}

void run_echo(){
    char text[256];
    printf("Enter text: ");

    getchar();

    if(fgets(text, sizeof(text), stdin) != NULL){
        printf("%s\n", text);
    }
}