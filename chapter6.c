// polish notation : an arithmetic notation where the operator comes before the operand 
//eg: 1+2+3 : +1 2 3 or 6+(2*9) : + 6 (* 2 9)
#include "mpc/mpc.h"
#ifdef _WIN32
static char buffer[2048];
char *readline(char* prompt){
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char *cpy = malloc(strlen(buffer) + 1);
    strcpy(cpy, buffer);  // Copy the buffer correctly
    cpy[strlen(cpy) - 1] = '\0';
    return cpy;
}
void add_history(char *unused) {}

#else
#include<editline/readline.h>
#include<editline/history.h>
#endif



int main(int argc , char ** argv){
    // Create some Parsers 
    mpc_parser_t* Number=mpc_new("number");
    mpc_parser_t* Operator=mpc_new("operator");
    mpc_parser_t* Expr=mpc_new("expr");
    mpc_parser_t* Lispy=mpc_new("lispy");

    // define them with language
    mpca_lang(MPCA_LANG_DEFAULT,
    "                                           \
    number : /-?[0-9]+/;                         \
    operator : '+'| '-' | '*' | '/';              \
    expr: <number> | '(' <operator> <expr>+')' ;   \
    lispy : /^/ <operator> <expr>+ /$/;             \
    ",
    
    Number,Operator,Expr,Lispy);
     puts("Lispy version 0.0.0.0.2");
     puts("Bhagna ho to ctrl + c daba aur nikal");
    
    while(1){
        char * input = readline("lispy>");
        add_history(input);

        // Attempt to parse the user output
        mpc_result_t r;
        if(mpc_parse("<stdin>",input,Lispy,&r)){
         // if successful print and delete the AST
         mpc_ast_print(r.output);
         mpc_ast_delete(r.output);
        }
        else{
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
            
        }
        free(input);
    }

    // undefine and delete our parsers 
    mpc_cleanup (4,Number,Operator,Expr,Lispy);

    return 0;
}
// AST = abstract syntax tree
// compilation used :cc -std=c99 -Wall chapter6.c mpc/mpc.c -lreadline -o a.out
//code running : ./a.out