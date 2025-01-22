#include "mpc/mpc.h"
#ifdef _WIN32
static char buffer[2048];
char* readline(char* prompt){
    fputs(prompt,stdout);
    fgets(buffer,2048,stdin);
    char *cpy =malloc(strlen(buffer)+1);
    strcpy(cpy,buffer);
    cpy[strlen(cpy)-1]='\0';
    return cpy;
}
void add_history (char * unused){}

#else 
#include<editline/readline.h>
#include<editline/history.h>
#endif
//Create Enumeration of possible Error types
enum{LERR_DIV_ZERO, LERR_BAD_OP,LERR_BAD_NUM};

//Create Enumeration of possible lval Types
enum{LVAL_NUM,LVAL_ERR};

//declare new lval struct
typedef struct{
    int type;
    long num;
    int err;
}lval;

// for this sinario a union will be more effecitve kyuki eak time pe ya to type ,num,ya err me se eak hi jayega
//equilvalent union
// typedef union{
//     long num;
//     long type;
//     int err;
// }lval;

//Create a new number type lval in a function jiska returntype lval hai
lval lval_num(long x){
    lval v;
    v.type=LVAL_NUM;
    v.num=x;
    return v;
}

//Create a new error type lval in a function jo lval type return karega
lval lval_err(int x){
    lval v;
    v.type= LVAL_ERR;
    v.err=x;
    return v;
}

//Print an "lval"

void lval_print(lval v){
    switch(v.type){
        // in case the type is a number print it
        //the "break" out of the switch

        case LVAL_NUM: printf("%li",v.num);{break;}

        // in the case of an error 
        case LVAL_ERR:
        //check what type of error it is and print it
            if(v.err == LERR_DIV_ZERO){
                printf("Error : Division by zero");
            }
            if(v.err==LERR_BAD_OP){
                printf("Error :  invalid Operator");
            }
            if(v.err==LERR_BAD_NUM){
                printf("ERROR : INVALID NUMBER");
            }
            break;
    }
}
//Print an "lval" followed by a newline
void lval_println(lval v){
    lval_print(v);
    putchar('\n');
}
lval eval_op(lval x,char* op, lval y){
    //if either the value is error return it
    if(x.type==LVAL_ERR){ return x; }
    if(y.type==LVAL_ERR){ return y; }
    //otherwise do maths on the number values

    if(strcmp(op,"+")==0){ return lval_num(x.num+y.num);}
    if(strcmp(op,"-")==0){ return lval_num(x.num-y.num);}
    if(strcmp(op,"*")==0){ return lval_num(x.num*y.num);}
    if(strcmp(op,"/")==0){ 
        //if second operand is zero return error
        //use of ternery operator
        return y.num==0
            ? lval_err(LERR_DIV_ZERO)
            :lval_num(x.num/y.num);
    }
    if(strcmp(op,"%")==0){ return lval_num(x.num%y.num);}// modulo operation to get the remainder
    return lval_err(LERR_BAD_OP);
}
lval eval(mpc_ast_t* t){
    if(strstr(t->tag,"number")){
        //check if there is some error in conversion
        errno=0;
        long x=strtol(t->contents,NULL,10);
        return errno!=ERANGE? lval_num(x): lval_err(LERR_BAD_NUM);
    }
    char *op =t->children[1]->contents;
    lval x=eval(t->children[2]);
    
    int i =3;
    while(strstr(t->children[i]->tag,"expr")){
        x=eval_op(x,op,eval(t->children[i]));
        i++;
    }
    return x;
}

int main(int argc,char ** argv){
    mpc_parser_t* Number=mpc_new("number");
    mpc_parser_t* Operator=mpc_new("operator");
    mpc_parser_t* Expr=mpc_new("expr");
    mpc_parser_t* Lispy=mpc_new("lispy");


    mpca_lang(MPCA_LANG_DEFAULT,
    "\
    number : /-?[0-9]+/;\
    operator: '+'|'-'|'*'|'/'|'%';\
    expr : <number> |'(' <operator><expr>+ ')';\
    lispy : /^/ <operator> <expr> +/$/;\
    ",
    Number,Operator,Expr,Lispy);


    puts("Lispy Version 0.0.0.0.4");
    puts("Agar galti se aagaya to Ctrl+C daba aur nikal jaa");

    while(1){
        char *input=readline("lispy>");
        add_history(input);

        mpc_result_t r;
        if(mpc_parse("<stdin>",input,Lispy,&r)){
            lval result =eval(r.output);
            lval_println(result);
            mpc_ast_delete(r.output);
        }
        else{
            mpc_err_print(r.error);
            mpc_err_delete(r.error);

        }
        free(input);
    }
    mpc_cleanup(4,Number,Operator,Expr,Lispy);
    return 0;
}
//cc -std=c99 -Wall chapter8.c mpc/mpc.c -lreadline -o a.out