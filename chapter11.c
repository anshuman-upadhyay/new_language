#include "mpc/mpc.h"

#ifdef _WIN32

static char buffer[2048];
char * readline(char* prompt){
    fputs(prompt,stdout);
    fgets(buffer,2048,stdin);
    char * cpy=malloc(strlen(buffer)+1);
    strcpy(cpy,buffer);
    cpy[strlen(cpy)-1]='\0';
    return cpy;
}
void add_history(char* unused){}
#else
#include<editline/readline.h>
#include<editline/history.h>
#endif

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;
//add QEXPR as possible lval type

enum { LVAL_ERR, LVAL_NUM, LVAL_SYM,
        LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR };

typedef lval*(*lbuiltin)(lenv*,lval*);
typedef struct lval{
    int type;
    long num;
    char* err;
    char* sym;
    lbuiltin fun;
    int count;
    lval** cell;
}lval;
// Constructors for different types in the ENUM
// LVAL_NUM
lval*lval_num(long x){
    lval* v=malloc(sizeof(lval));
    v->type=LVAL_NUM;
    v->num=x;
    return v;
}
// LVAL_ERR
lval* lval_err(char* fmt,...){
    lval*v=malloc(sizeof(lval));
    v->type=LVAL_ERR;
    //Create a va list and initialize it
    va_list va;
    va_start(va,fmt);
    // Allocate 512 byte of space
    v->err=malloc(512);
    //printf the error string with a maximum of 511 characters
    vsnprintf(v->err,511,fmt,va);
    //reallocate to number of bytes actually used 
    v->err=realloc(v->err,strlen(v->err)+1);
    //cleanup our va list
    va_end(va);
    return v;
}
// LVAL_SYM
lval* lval_sym(char* s){
    lval* v=malloc(sizeof(lval));
    v->type=LVAL_SYM;
    v->sym=malloc(strlen(s)+1);
    strcpy(v->sym,s);
    return v;
}
// LVAL_FUN
lval* lval_fun(lbuiltin func){
    lval* v= malloc(sizeof(lval));
    v->type=LVAL_FUN;
    v->fun=func;
    return v;
}
// LVAL_SEXPR
lval* lval_sexpr(void){
    lval* v=malloc(sizeof(lval));
    v->type=LVAL_SEXPR;
    v->count=0;
    v->cell=NULL;
    return v;
}
// LVAL_QEXPR
lval* lval_qexpr(void){
    lval*v= malloc(sizeof(lval));
    v->type=LVAL_QEXPR;
    v->count=0;
    v->cell=NULL;
    return v;
}

void lval_del(lval* v){
    switch(v->type){
        case LVAL_NUM:break;
        case LVAL_ERR : free(v->err);break;
        case LVAL_SYM: free(v->sym);break;
        case LVAL_FUN: break;
        //if Qexpr or sexpr then delete all elements inside
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            for(int i=0;i< v->count;i++){
                lval_del(v->cell[i]);
            }
            //also free the memory allocated to contain the pointers
            free(v->cell);
        break;
    }
    free(v);
}

lval* lval_copy(lval* v){
    lval* x=malloc(sizeof(lval));
    x->type=v->type;
    switch(v->type){
        //Copy Functions and numbers directly
        case LVAL_FUN: x->fun= v->fun;break;
        case LVAL_NUM: x->num= v->num;break;

        //for strings we allocate and copy
        case LVAL_ERR:
            x->err=malloc(strlen(v->err)+1);
            strcpy(x->err,v->err);break;

        case LVAL_SYM:
            x->sym=malloc(strlen(v->sym)+1);
            strcpy(x->sym,v->sym);break;

        // Copy list by copying each sub exp
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            x->count=v->count;
            x->cell=malloc(sizeof(lval*) * x->count);
            for(int i=0;i<x->count;i++){
                x->cell[i]=lval_copy(v->cell[i]);
            }
            break;
    }
    return x;
}

lval* lval_add(lval* v,lval* x){
    v->count++;
    v->cell=realloc(v->cell,sizeof(lval*) * v->count);
    v->cell[v->count-1]=x;
    return v;
}

lval* lval_pop(lval* v,int i){
    lval* x=v->cell[i];
    memmove(&v->cell[i],&v->cell[i+1],
    sizeof(lval*)* (v->count-i-1));
    v->count--;
    v->cell=realloc(v->cell,sizeof(lval*)* v->count);
    return x;
}

lval* lval_join(lval* x,lval*y){
    while(y->count){
        x=lval_add(x,lval_pop(y,0));
    }
    lval_del(y);
    return x;
}

lval* lval_take(lval* v,int i){
    lval* x=lval_pop(v,i);
    lval_del(v);
    return x;
}

void lval_print(lval* v);
void lval_print_expr(lval* v,char open,char close){
    putchar(open);
    for(int i=0;i<v->count;i++){
        lval_print(v->cell[i]);
        if(i!=(v->count-1)){
            putchar(' ');
        }
    }
    putchar(close);
}

void lval_print(lval* v){
    switch(v->type){
        case LVAL_NUM: printf("%li",v->num);break;
        case LVAL_ERR: printf("ERROR: %s",v->err);break;
        case LVAL_SYM: printf("%s",v->sym);break;
        case LVAL_FUN: printf("<function>");break;
        case LVAL_SEXPR: lval_print_expr(v,'(',')');break;
        case LVAL_QEXPR: lval_print_expr(v,'{','}');break;
    }
}

void lval_println(lval* v){ lval_print(v); putchar('\n');}

char * ltype_name(int t){
    switch(t){
        case LVAL_FUN: return "Function";
        case LVAL_NUM: return "Number";
        case LVAL_ERR: return "Error";
        case LVAL_SYM: return "Symbol";
        case LVAL_SEXPR: return "S-Expression";
        case LVAL_QEXPR: return "Q-Expression";
        default: return "Unkown";
    }
}



//lisp enviornment
struct lenv{
    int count;
    char** syms;
    lval** vals;
};

lenv* lenv_new(void){
    //Initialize struct
    lenv* e=malloc(sizeof(lenv));
    e->count=0;
    e->syms=NULL;
    e->vals=NULL;
    return e;
}

void lenv_del(lenv* e){
    //iterates over all element in enviornment and deletes them
    for(int i =0;i<e->count;i++){
        free(e->syms[i]);
        lval_del(e->vals[i]);
    }
    //frees allocated memory for lists 
    free(e->syms);
    free(e->vals);
    free(e);
}
lval* lenv_get(lenv* e,lval* k){
    //iterates over all items in enviornment
    for(int i =0;i<e->count;i++){
        //checks if the stored string matches the symbol string 
        //if it does returns a copy of the value 
        if(strcmp(e->syms[i],k->sym)==0){
            return lval_copy(e->vals[i]);
        }
    }
    //if no symbol found returns an error
    return lval_err("Unbound symbol '%s' ",k->sym);
}
void lenv_put(lenv* e ,lval* k,lval* v){
    //iterates over all items in enviornment
    //this is to see if variable already exists
    for(int i=0;i<e->count;i++){
        //if found delete the old one and rewrite it to a user given one
        if(strcmp(e->syms[i],k->sym)==0){
            lval_del(e->vals[i]);
            e->vals[i]=lval_copy(v);
            return;
        }
    }
    //if no existing memory allocate new to store data
    e->count ++;
    e->vals= realloc(e->vals,sizeof(lval*)* e->count);
    e->syms= realloc(e->syms,sizeof(char*)* e->count);

    //copy the contents of lval and symbol into new location
    e->vals[e->count-1]=lval_copy(v);
    e->syms[e->count-1]=malloc(strlen(k->sym)+1);
   strcpy(e->syms[e->count-1],k->sym);

}


//Builtins


// Macro: Preprocessor statements that are evaluated before the code is compiled
//here LASSERT is a macro
#define LASSERT(args,cond,fmt,...)\
    if(!(cond)){lval* err =lval_err(fmt,##__VA_ARGS__); lval_del(args);return err;}

#define LASSERT_TYPE(func,args,index,expect)\
    LASSERT(args,args->cell[index]->type==expect,\
    "Function '%s' passed incorrect type for argument %i. Got: %s,Expected: %s .",\
    func,index, ltype_name(args->cell[index]->type),ltype_name(expect))
#define LASSERT_NUM(func,args,nums)\
    LASSERT(args,args->count==num,\
    "Function '%s' passed incorrect number of arguments. Got: %i, Expected: %i.",\
    func,args->count,num)
#define LASSERT_NOT_EMPTY(func,args,index)\
    LASSERT(args,args->cell[index]->count!=0,\
    "Function '%s' passed {} for argument %i."func,index)



lval* lval_eval(lenv* e,lval* v);
//Takes one or more arguments and returns a new Q Exp containing args
lval* builtin_list(lenv* e,lval* a){
    a->type=LVAL_QEXPR;
    return a;
}
// takes a Q-exp and returns a Q-exp with only of the first element
lval* builtin_head(lenv* e,lval* a){
    LASSERT(a,a->count==1,
    "Function 'head' passed too many arguments. "
    "Got %i ,Expected %i.",a->count,1
    );
    LASSERT(a,a->cell[0]->type==LVAL_QEXPR,
    "Function 'head' passed incorrect type");
    LASSERT(a,a->cell[0]->count!=0,
     "Function 'head' passed {}.");

    lval* v=lval_take(a,0);
    while(v->count>1){
        lval_del(lval_pop(v,1));
    }
    return v;
}
//takes a Qexp and returns a Qexp with the first element removed
lval* builtin_tail(lenv* e,lval* a){
     LASSERT(a,a->count==1,
    "Function 'tail' passed too many arguments. "
    "Got %i ,Expected %i.",a->count,1
    );
    LASSERT(a,a->cell[0]->type==LVAL_QEXPR,
    "Function 'tail' passed incorrect type");
    LASSERT(a,a->cell[0]->count!=0,
     "Function 'tail' passed {}.");
    lval* v=lval_take(a,0);
    lval_del(lval_pop(v,0));
    return v;
}
// takes a Q exp and evaluates it as if it were a S exp
lval* builtin_eval(lenv* e,lval*a){
    LASSERT(a,a->count==1,"Function 'eval' passed too many arguments.");
    LASSERT(a,a->cell[0]->type==LVAL_QEXPR,"Function 'eval' passed incorrect type");
    lval* x=lval_take(a,0);
    x->type= LVAL_SEXPR;
    return lval_eval(e,x);
}
//takes one or more Qexp and returns a Qexpression of them conjoined together
lval* builtin_join(lenv*e,lval* a){
    for(int i =0;i<a->count;i++){
        LASSERT(a,a->cell[i]->type==LVAL_QEXPR,"Function 'join' passed incorrect type");
    }
    lval* x=lval_pop(a,0);
    while(a->count){
        x=lval_join(x,lval_pop(a,0));
    }
    lval_del(a);
    return x;
}


lval* builtin_op(lenv* e,lval* a,char* op){
    for(int i=0;i<a->count;i++){
        if(a->cell[i]->type!=LVAL_NUM){
            lval_del(a);
            return lval_err("Cannot operate on non number");
        }
    }
    lval* x=lval_pop(a,0);
    if((strcmp(op,"-")==0)&& a->count==0){ x->num=-x->num;}
    while(a->count>0){
        lval* y =lval_pop(a,0);
        if(strcmp(op,"+")==0){ x->num += y->num;}
        if(strcmp(op,"-")==0){ x->num -= y->num;}
        if(strcmp(op,"*")==0){ x->num *= y->num;}
        if(strcmp(op,"%")==0){ x->num = x->num % y->num;}

        if(strcmp(op,"/")==0){
            if(y->num==0){
                lval_del(x);lval_del(y);
                x=lval_err("Division by zero ");
                break;
            }
            x->num/=y->num;
        }
        lval_del(y);

    }
    lval_del(a);
    return x;
}

lval* built_add(lenv* e,lval* a){
    return builtin_op(e,a,"+");
}
lval* built_sub(lenv* e,lval* a){
    return builtin_op(e,a,"-");
}
lval* built_mul(lenv* e,lval* a){
    return builtin_op(e,a,"*");
}
lval* built_div(lenv* e,lval* a){
    return builtin_op(e,a,"/");
}
lval* builtin_def(lenv* e,lval* a){
    LASSERT(a,a->cell[0]->type==LVAL_QEXPR,
    "Function 'def' passed incorrect type!.");
    //First argument is symbol list
    lval* syms=a->cell[0];
    // Ensure all elements of first list are symbols
    for(int i =0;i<syms->count;i++){
        LASSERT(a,syms->cell[i]->type==LVAL_SYM,
        "Function 'def' cannot define non-symbol");
    }
    //check correct number of symbols and values 
    LASSERT(a,syms->count==a->count-1,
    "Function 'def' cannot define incorrect"
    "number of values to symbols");
    //Assign copies of values to symbols
    for(int i=0;i<syms->count;i++){
        lenv_put(e,syms->cell[i],a->cell[i+1]);
    }
    lval_del(a);
    return lval_sexpr();
}
void lenv_add_builtin(lenv* e, char * name,lbuiltin func){
    lval* k=lval_sym(name);
    lval* v=lval_fun(func);
    lenv_put(e,k,v);
    lval_del(k);lval_del(v);
}

void lenv_add_builtins(lenv* e){
    //Variable Function
    lenv_add_builtin(e,"def",builtin_def);                  
    //lsit functions 
    lenv_add_builtin(e,"list",builtin_list);
    lenv_add_builtin(e,"head",builtin_head);
    lenv_add_builtin(e,"tail",builtin_tail);
    lenv_add_builtin(e,"eval",builtin_eval);
    lenv_add_builtin(e,"join",builtin_join);

    //Mathematical Function 
    lenv_add_builtin(e,"+",built_add);
    lenv_add_builtin(e,"-",built_sub);
    lenv_add_builtin(e,"*",built_mul);
    lenv_add_builtin(e,"/",built_div);
}
//evlauations
lval* lval_eval_sexpr(lenv* e, lval*v){
    for(int i =0;i<v->count ;i++){
        v->cell[i]=lval_eval(e,v->cell[i]);
    }
    for(int i =0;i<v->count;i++){
        if(v->cell[i]->type==LVAL_ERR){return lval_take(v,i);}
    }
    if(v->count==0){return v;}
    if(v->count==1){return lval_take(v,0);}

    // Ensure first element is a function after evaluation
    lval* f=lval_pop(v,0);
    if(f->type!=LVAL_FUN){
        lval_del(v);lval_del(f);
        return lval_err("First element is not a function");
    }
    // if so call function to get result
    lval* result = f->fun(e,v);
    lval_del(f);
    return result;

}
lval* lval_eval(lenv* e,lval* v){
    if(v->type==LVAL_SYM){
        lval*x=lenv_get(e,v);
        lval_del(v);
        return x;
    }
    if(v->type == LVAL_SEXPR){return lval_eval_sexpr(e,v);}
    return v;
}
lval* lval_read_num(mpc_ast_t* t){
    errno=0;
    long x= strtol(t->contents,NULL,10);
    return errno!=ERANGE ? lval_num(x) : lval_err("invalid number");
}

lval* lval_read(mpc_ast_t* t){
    if(strstr(t->tag,"number")){return lval_read_num(t);}
    if(strstr(t->tag,"symbol")){return lval_sym(t->contents);}

    lval* x= NULL;
    if(strcmp(t->tag,">")==0){ x=lval_sexpr();}
    if(strstr(t->tag,"sexpr")){ x=lval_sexpr();}
    if(strstr(t->tag,"qexpr")){ x=lval_qexpr();}

    for(int i =0;i<t->children_num;i++){
        if(strcmp(t->children[i]->contents,"(")==0){continue;}
        if(strcmp(t->children[i]->contents,")")==0){continue;}
        if(strcmp(t->children[i]->contents,"{")==0){continue;}
        if(strcmp(t->children[i]->contents,"}")==0){continue;}
        if(strcmp(t->children[i]->tag,"regex")==0){continue;}
        x=lval_add(x,lval_read(t->children[i]));
    }
    return x;
}

























int main(int argc,char** argv){
    mpc_parser_t* Number=mpc_new("number");
    mpc_parser_t* Symbol=mpc_new("symbol");
    mpc_parser_t* Sexpr=mpc_new("sexpr");
    mpc_parser_t* Qexpr=mpc_new("qexpr");
    mpc_parser_t* Expr=mpc_new("expr");
    mpc_parser_t* Lispy=mpc_new("lispy");
    mpca_lang(MPCA_LANG_DEFAULT,
    "                                               \
    number : /-?[0-9]+/;                            \
    symbol:  /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;        \
    sexpr:'('<expr>* ')';                           \
    qexpr : '{' <expr>* '}';                        \
    expr: <number>| <symbol>|<sexpr>| <qexpr>;      \
    lispy: /^/ <expr>* /$/;                         \
    ",
    Number,Symbol,Sexpr,Qexpr,Expr,Lispy);

    puts("Lispy Version 0.0.0.0.6");
    puts("Press Ctrl+c to Exit\n");

    lenv* e=lenv_new();
    lenv_add_builtins(e);

    while(1){
        char* input=readline("lispy> ");
        add_history(input);
        mpc_result_t r;
        if(mpc_parse("<stdin> ",input,Lispy,&r)){
            lval*x =lval_eval(e,lval_read(r.output));
            lval_println(x);
            lval_del(x);
            mpc_ast_delete(r.output);
        }
        else{
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input);
    }
    mpc_cleanup(6,Number,Symbol,Sexpr,Qexpr,Expr,Lispy);
    return 0;
}

