#include<stdio.h>
#include<stdlib.h>
//ye do line are commom for every code : used to import standard input files and libraries
// from here the real shit begins 
#ifdef _WIN32
#include<string.h>
static char buffer[2048];
//a fake readline function
char * readline(char*prompt){
    fputs(prompt,stdout);
    fgets(buffer,2048,stdin);
    strcpy(cpy,buffer);
    cpy[strlen(cpy)-1]='\0';
    return cpy;
}
// fake history function
void add_history(char* unused){}
//otherwise include the editline headers
#else
#include<editline/readline.h>
#include<editline/history.h>
#endif
#include"mpc/mpc.h"
int main(int argc, char* argv[]){
    puts("lispy version 0.0.0.1 this time aapke pyaare host ke saath" );
    puts("agar dumnahi hai aur input ka tho  ctrl+C daba aur nikal jaa \n");
    while(1){
        char * input=readline("Bhok>:");
        add_history(input);
        printf("teri jaat ka paida maaru saale %s\n",input);
        free(input);

    }
    return 0;
}
// iss particular code ke liye main function ka need nahi tha 
// yaha pe 3 keywords dekh #ifdef,#else,#endif : ye teeno milke code ko multiple operating system pe use kiye jaane ke liye useable banare hai 
// ye teeno : eak preprocessor hai jo if else and end ki tarah kaam kar rahe hai 
// ie : #ifdef:agar win32 hua tho 
// // fake history function//a fake readline function ka use karega 
// #else : nagar win32 nai hua then woh readline aur addhistory wale headers ko run karega aur import and use karega 
// #endif: basically ye sab cheez ka end ho gya bata rah hai 