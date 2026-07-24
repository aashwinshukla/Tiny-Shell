#include<stdio.h>
#include<string.h>

char command[10];

int main(){
    printf("\n--------------Tiny Shell--------------\n");
    printf("\n             WELCOME USER             \n");
    printf("Enter your command:\n");
    printf("[OR /.help]\n");
    scanf(" %s", command);
    return 0;
}