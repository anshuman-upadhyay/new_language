// chapters covered in this onr:1,2,3,4
#include<stdio.h>
#include<stdlib.h>
// due to absense of source file this header cannot be used (day two pe achanak se cbal gaya )
#include<editline/readline.h>
#include<editline/history.h>

typedef struct{
    float s;
    int u;
}point;
int add(int *a,int *b){
    return *a + *b;

}
static char input[2048];

// chapter 2
int main(int argc, char** argv){
    puts("Helloworld");//short for put string used only for strings 
    //puts("%d",5);//Error shown : too many arguments mane ye pnly single string as arguments leta hai 


//chapter 3
    int a=3;
    int b=3;
    int c=add(&a,&b);
    printf("%d\n",c);
    for(int i =0;i<3;i++){
        printf("ha mai hi hu woh\n");
    }


//chapter 4
    puts("LISPY version 0.0.0.0.1\n");
    puts("Press CTRL+C to exit now\n");
    while(1){
        // fputs("lispy>",stdout);
        // fgets(input,2048,stdin);
        //instead of fgets and fputs we can also use readline and add_history to achieve the same results 
        char * input=readline("lispy> ");
        add_history(input);

        printf("bharwe hai aap %s",input);
        free(input);
    }
//pg24


printf("\n");
    return 0;
}
//notes : 
//chapter 2:
//LISP:it is a family of programming languages characterised by the fact that all their computation is represented by lists
//gdb:GNU DEBUGGER:It allows developers to step through their program’s execution, inspect variables, and analyze program behavior, making it an essential tool for debugging and troubleshooting software issues
//puts: faster than printf but less versitile and gves an automatic new line , takes single string as argument 
//chapter 4:
// To output our prompt we use the function fputs . This is a slight variation on puts
// which does not append a newline character. We use the fgets function for getting
// user input from the command line. Both of these functions require some file to write
// to, or read from. For this we supply the special variables stdin and stdout . These
// are declared in <stdio.h> and are special file variables representing input to, and
// output from, the command line. When passed this variable the fgets function will
// wait for a user to input a line of text, and when it has it will store it into the input
// buffer, including the newline character. So that fgets does not read in too much
// data we also must also supply the size of the buffer 2048 .
