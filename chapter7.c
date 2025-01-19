// abstract syntax tree : represents the structure of the program based on the input given by the user
//At the leaves of this tree are numbers and operators - the actual data to be processed.
#include "mpc/mpc.h" 
#ifdef _WIN32
static char buffer[2048];
char * readline(char* prompt){
    fputs(prompt,stdout);
    fgets(buffer,2048,stdin);
    char* cpy=malloc(strlen(buffer)+1);
    strcpy(cpy,buffer);
    cpy[strlen(cpy)-1]='\0';
    return cpy;
}
void add_history(char* unused){}

#else
#include<editline/readline.h>
#include<editline/history.h>

#endif

long eval_op(long x,char* op,long y){
    //? strcmp : compares two char* and returns 0 if equal
    if(strcmp(op,"+")==0){return x+y;}
    if(strcmp(op,"-")==0){return x-y;}
    if(strcmp(op,"*")==0){return x*y;}
    if(strcmp(op,"/")==0){return x/y;}
    return 0;
}

long eval(mpc_ast_t* t){
    //if tagged as number return it directly
    //? strstr : takes two char* input and returns a pointer to the location of the second in the first or 0 if second is not a substring of first 
    if(strstr(t->tag,"number")){
        return atoi(t->contents);
        //?atoi = converts char* into long
    }
    //the operator is always second child
    char* op=t->children[1]->contents;
    //we store the third child in x
    long x=eval(t->children[2]);
    //iterate the remaining children and combining
    int i=3;
    while (strstr(t->children[i]->tag,"expr")){
        x=eval_op(x,op,eval(t->children[i]));
        i++;
    }
    return x;
}
int main(int argc ,char**argv){
    mpc_parser_t* Number=mpc_new("number");
    mpc_parser_t* Operator=mpc_new("operator");
    mpc_parser_t* Expr=mpc_new("expr");
    mpc_parser_t* Lispy=mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
    "                                           \
    number : /-?[0-9]+/;                        \
    operator : '+'| '-'| '*'| '/';              \
    expr : <number> | '('<operator> <expr>+ ')' ;\
    lispy : /^/ <operator> <expr> + /$/;\
    ",
    Number,Operator,Expr,Lispy);

    puts("Library Version 0.0.0.0.3");
    puts("ctrl +c daba aur gand mara ");

    while(1){
        char *input =readline("Lispy>");
        add_history(input);
        mpc_result_t r;
        if(mpc_parse("<stdin>",input ,Lispy,&r)){
            long result =eval(r.output);
            printf("%li\n",result);
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




// internal structure of mpc_ast_t
// ~typedef struct mpc_ast_t{
// ~    char * tag;
// ~    char * contents;
// ~    mpc_state_t state;
//  ~   int children_nums;
// ~    struct mpc_ast_t ** children;
// ~}mpc_ast_t;
// syntax to access the field in mpc_ast_t
//? to load AST from output
//~ mpc_ast_t* a=r.output;
//~ printf("Tag: %s\n",a->tag);
//~ printf("Contents: %s\n",a->contents);
//~ printf("Number of children : %i\n",a->children_num);
//? get first child 
//~ mpc_ast_t* c0=a->children[0];
// ~printf("First child Tag : %s\n",c0->tag);
//~ printf("First child contents : %s\n",c0->contents);
// ~printf("First child number of children: %i\n",c0->children_num);


// for recursion
//?find the number of nodes childre of the tree has
// ~int number_of_node(mpc_ast_t* t){
// ~    if(t->children_num==0){ return 1;}
// ~    if(t->children_num>=1){
// ~        int total =1;
// ~        for(int i=0;i<t->children_num;i++){
// ~            total=total+number_of_node(t->children[i]);
//  ~       }
//  ~       return total;
//  ~   }
// ~}