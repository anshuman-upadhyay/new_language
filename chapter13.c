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


//Lisp VALUES
enum { LVAL_ERR, LVAL_NUM, LVAL_SYM,
        LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR };

typedef lval*(*lbuiltin)(lenv*,lval*);
typedef struct lval{
    int type;
    //Basic
    long num;
    char* err;
    char* sym;
    // Function
    lbuiltin builtin;// renamed fun to builtin 
    lenv* env;
    lval* formals;
    lval*body;
    // Expression
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
lval* lval_builtin(lbuiltin func){
    lval* v= malloc(sizeof(lval));
    v->type=LVAL_FUN;
    v->builtin=func;
    return v;
}

lenv* lenv_new(void);
// LVAL_LAMBDA
lval* lval_lambda(lval* formals,lval* body){
    lval* v =malloc(sizeof(lval));
    v->type=LVAL_FUN;
    //Set Builtin to NULL
    v->builtin=NULL;
    //Build new enviornment
    v->env=lenv_new();
    //set formals and body
    v->formals=formals;
    v->body=body;
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

void lenv_del(lenv* e);

void lval_del(lval* v){
    switch(v->type){
        case LVAL_NUM:break;
        case LVAL_ERR : free(v->err);break;
        case LVAL_SYM: free(v->sym);break;
        case LVAL_FUN: 
            if(!v->builtin){
                lenv_del(v->env);
                lval_del(v->formals);
                lval_del(v->body);
            }
            break;
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

lenv* lenv_copy(lenv* e);

lval* lval_copy(lval* v){
    lval* x=malloc(sizeof(lval));
    x->type=v->type;
    switch(v->type){
        //Copy Functions if not null
        case LVAL_FUN: 
            if(v->builtin){
                x->builtin= v->builtin;
            }
            else{
                x->builtin=NULL;
                x->env=lenv_copy(v->env);
                x->formals=lval_copy(v->formals);
                x->body=lval_copy(v->body);
            }
            break;
            //copy numbers directly
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

lval* lval_join(lval* x,lval*y){
    for(int i =0;i<y->count;i++){
        x=lval_add(x,y->cell[i]);
    }
    free(y->cell);
    free(y);
    return x;
}


lval* lval_pop(lval* v,int i){
    lval* x=v->cell[i];
    memmove(&v->cell[i],&v->cell[i+1],
    sizeof(lval*)* (v->count-i-1));
    v->count--;
    v->cell=realloc(v->cell,sizeof(lval*)* v->count);
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
        case LVAL_FUN: 
            if(v->builtin){
                printf("<builtin>");
            }
            else{
                printf("(\\");lval_print(v->formals);
                putchar(' ');lval_print(v->body);putchar(')');
            }
        break;
        case LVAL_SEXPR: lval_print_expr(v,'(',')');break;
        case LVAL_QEXPR: lval_print_expr(v,'{','}');break;
    }
}

void lval_println(lval* v){ lval_print(v); putchar('\n');}

//Equality checker function

int lval_eq(lval* x,lval* y){
    //Different types are always unequal
    if(x->type!= y->type){ return 0;}
    //Compare based upon type
    switch( x->type){
        // compare Number values
        case LVAL_NUM : return (x->num==y->num);
        //compare String Values
        case LVAL_ERR : return (strcmp(x->err,y->err)==0);
        case LVAL_SYM : return (strcmp(x->sym,y->sym)==0);

        //if builtin compare,otherwise compare formals and body
        case LVAL_FUN:
            if(x->builtin || y->builtin){
                return x->builtin == y->builtin;
            }
            else{
                return lval_eq(x->formals,y->formals)&&
                lval_eq(x->body,y->body);
            }
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            if(x->count!= y->count){return 0;}
            for(int i =0;i<x->count;i++){
                //if any element not equal then whole list in not equal
                if(!lval_eq(x->cell[i],y->cell[i])){ return 0;}
            }
            // otherwise the lists must be equal
            return 1;
            break;
    }
            return 0;
}

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
    lenv* par;
    int count;
    char** syms;
    lval** vals;
};

lenv* lenv_new(void){
    //Initialize struct
    lenv* e=malloc(sizeof(lenv));
    e->par=NULL;
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
lenv* lenv_copy(lenv* e){
    lenv* n=malloc(sizeof(lenv));
    n->par=e->par;
    n->count=e->count;
    n->syms=malloc(sizeof(char* )* n->count);
    n->vals=malloc(sizeof(lval* )* n->count);
    for(int i=0;i<e->count;i++){
        n->syms[i]=malloc(strlen(e->syms[i])+1);
        strcpy(n->syms[i],e->syms[i]);
        n->vals[i]=lval_copy(e->vals[i]);
    }
    return n;

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
    //if no symbol found check in parents other wise error
    if(e->par){
        return lenv_get(e->par,k);
    }
    else{
        return lval_err("Unbound symbol '%s' ",k->sym);
    }
    
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

void lenv_def(lenv* e,lval* k,lval* v){
    //iterate till e has no parent
    while(e->par){e=e->par;}
    //put values in e;
    lenv_put(e,k,v);
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
#define LASSERT_NUM(func,args,num)\
    LASSERT(args,args->count==num,\
    "Function '%s' passed incorrect number of arguments."\
    " Got: %i, Expected: %i.",\
    func,args->count,num)
#define LASSERT_NOT_EMPTY(func,args,index)\
    LASSERT(args,args->cell[index]->count!=0,\
    "Function '%s' passed {} for argument %i."func,index)



lval* lval_eval(lenv* e,lval* v);


lval* builtin_lambda(lenv* e,lval* a){
    //Check two arguments each of which are Q-Expressions
    LASSERT_NUM("\\",a,2);
    LASSERT_TYPE("\\",a,0,LVAL_QEXPR);
    LASSERT_TYPE("\\",a,1,LVAL_QEXPR);

    // check first Q-Expression contains only Symbols
    for(int i=0;i<a->cell[0]->count;i++){
        LASSERT(a,(a->cell[0]->cell[i]->type==LVAL_SYM),
        ltype_name(a->cell[0]->cell[i]->type),ltype_name(LVAL_SYM));
    }
    //Pop first two arguments and pass them to lval_lambda
    lval* formals=lval_pop(a,0);
    lval* body =lval_pop(a,0);
    lval_del(a);
    return lval_lambda(formals,body);
}



//Takes one or more arguments and returns a new Q Exp containing args
lval* builtin_list(lenv* e,lval* a){
    a->type=LVAL_QEXPR;
    return a;
}
// takes a Q-exp and returns a Q-exp with only of the first element
lval* builtin_head(lenv* e,lval* a){
    LASSERT_NUM("head",a,1);
    LASSERT_TYPE("head",a,0,LVAL_QEXPR);
    LASSERT_NOT_EMPTY("head",a,0);

    lval* v=lval_take(a,0);
    while(v->count>1){
        lval_del(lval_pop(v,1));
    }
    return v;
}
//takes a Qexp and returns a Qexp with the first element removed
lval* builtin_tail(lenv* e,lval* a){
    LASSERT_NUM("tail",a,1);
    LASSERT_TYPE("tail",a,0,LVAL_QEXPR);
    LASSERT_NOT_EMPTY("tail",a,0);
    lval* v=lval_take(a,0);
    lval_del(lval_pop(v,0));
    return v;
}
// takes a Q exp and evaluates it as if it were a S exp
lval* builtin_eval(lenv* e,lval*a){
    LASSERT_NUM("eval",a,1);
    LASSERT_TYPE("eval",a,0,LVAL_QEXPR);
    lval* x=lval_take(a,0);
    x->type =LVAL_SEXPR;
    return lval_eval(e,x);
}
//takes one or more Qexp and returns a Qexpression of them conjoined together
lval* builtin_join(lenv*e,lval* a){
    for(int i =0;i< a->count;i++){
        LASSERT_TYPE("join",a,i,LVAL_QEXPR);
    }
    lval* x=lval_pop(a,0);
    while(a->count){
        lval* y =lval_pop(a,0);
        x= lval_join(x,y);
    }
    lval_del(a);
    return x;
}


lval* builtin_op(lenv* e,lval* a,char* op){
    for(int i =0;i<a->count;i++){
        LASSERT_TYPE(op,a,i,LVAL_NUM);
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

lval* builtin_var(lenv* e,lval* a,char* func){
    LASSERT_TYPE(func,a,0,LVAL_QEXPR);
    lval* syms= a->cell[0];
    for(int i=0;i<syms->count;i++){
        LASSERT(a,(syms->cell[i]->type==LVAL_SYM),
        "Function '%s'cannot define non-Symbols "
        "Got :%s , Expected : '%s'",func,
        ltype_name(syms->cell[i]->type),ltype_name(LVAL_SYM)
        );
    }
    LASSERT(a,(syms->count==a->count-1),
    "Function '%s' passed too many arguments for symbols "
    "Got : '%i', Expected: '%i'",func,syms->count,a->count-1
    );

    for(int i=0;i<syms->count;i++){
        //if 'def' defined in globally. If'put' defined in locally
        if(strcmp(func,"def")==0){
            lenv_def(e,syms->cell[i],a->cell[i+1]);
        }
        if(strcmp(func,"=")==0){
            lenv_put(e,syms->cell[i],a->cell[i+1]);
        }
    }
    lval_del(a);
    return lval_sexpr();
}

lval* builtin_def(lenv* e,lval* a){
    return builtin_var(e,a,"def");
}
lval* builtin_put(lenv* e,lval* a){
    return builtin_var(e,a,"=");
}
 
//A funtions to allow comaprisons this only work on number types 
lval* builtin_ord(lenv* e,lval*a,char* op){
    LASSERT_NUM(op,a,2);
    LASSERT_TYPE(op,a,0,LVAL_NUM);
    LASSERT_TYPE(op,a,1,LVAL_NUM);

    int r;
    if(strcmp(op,">")==0){
        r=(a->cell[0]->num > a->cell[1]->num);
    }
    if(strcmp(op,"<")==0){
        r=(a->cell[0]->num < a->cell[1]->num);
    }
    if(strcmp(op,">=")==0){
        r=(a->cell[0]->num >= a->cell[1]->num);
    }
    if(strcmp(op,"<=")==0){
        r=(a->cell[0]->num <= a->cell[1]->num);
    }
    if(strcmp(op,"&&")==0){
        r=(a->cell[0]->num & a->cell[1]->num);
    }
    if(strcmp(op,"||")==0){
        r=(a->cell[0]->num | a->cell[1]->num);
    }
    lval_del(a);
    return lval_num(r);
}
// Greater than function for comparision
lval* builtin_gt(lenv* e,lval* a){
    return builtin_ord(e,a,">");
}
// Greater than equal to function for comparision
lval* builtin_ge(lenv* e,lval* a){
    return builtin_ord(e,a,">=");
}

//Lesser than function for comparision
lval* builtin_lt(lenv* e,lval* a){
    return builtin_ord(e,a,"<");
}
// Lesser than equal to function for comparision
lval* builtin_le(lenv* e,lval* a){
    return builtin_ord(e,a,"<=");
}
lval* builtin_band(lenv* e,lval* a){
    return builtin_ord(e,a,"&&");
}
lval* builtin_bor(lenv* e,lval* a){
    return builtin_ord(e,a,"||");
}


//Equality checker 
lval* builtin_cmp(lenv* e,lval* a,char *op){
    LASSERT_NUM(op,a,2);
    int r;
    if(strcmp(op,"==")==0){
        r=lval_eq(a->cell[0],a->cell[1]);
    }
    
    if(strcmp(op,"!=")==0){
        r=!lval_eq(a->cell[0],a->cell[1]);
    }
    lval_del(a);
    return lval_num(r);
    
}
lval* builtin_eq(lenv* e,lval* a){
    return builtin_cmp(e,a,"==");
}
lval* builtin_ne(lenv* e,lval* a){
    return builtin_cmp(e,a,"!=");
}
// Creating the builtin function for if operations
lval* builtin_if(lenv* e,lval* a){
    LASSERT_NUM("if",a,3);
    LASSERT_TYPE("if",a,0,LVAL_NUM);
    LASSERT_TYPE("if",a,1,LVAL_QEXPR);
    LASSERT_TYPE("if",a,2,LVAL_QEXPR);
    //Mark both expressions as evaluable 
    lval* x;
    a->cell[1]->type=LVAL_SEXPR;
    a->cell[2]->type=LVAL_SEXPR;

    if(a->cell[0]->num){
        //if condition is true evaluate first expression
        x=lval_eval(e,lval_pop(a,1));
    }
    else{
        //otherwise evaluate the second expression
        x=lval_eval(e,lval_pop(a,2));
    }

    //delete argument list and return
    lval_del(a);
    return x;

}

void lenv_add_builtin(lenv* e, char * name,lbuiltin func){
    lval* k=lval_sym(name);
    lval* v=lval_builtin(func);
    lenv_put(e,k,v);
    lval_del(k);lval_del(v);
}

void lenv_add_builtins(lenv* e){
    //Variable Function
    lenv_add_builtin(e,"\\",builtin_lambda);                  
    lenv_add_builtin(e,"def",builtin_def);                  
    lenv_add_builtin(e,"=",builtin_put);                  
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

    //Comparision functions 
    lenv_add_builtin(e,"if",builtin_if);
    lenv_add_builtin(e,"==",builtin_eq);
    lenv_add_builtin(e,"!=",builtin_ne);
    lenv_add_builtin(e,">",builtin_gt);
    lenv_add_builtin(e,"<",builtin_lt);
    lenv_add_builtin(e,">=",builtin_ge);
    lenv_add_builtin(e,"<=",builtin_le);
    //binary functions
    lenv_add_builtin(e,"&&",builtin_band);
    lenv_add_builtin(e,"||",builtin_bor);
}
//evlauations

lval* lval_call(lenv* e, lval* f,lval* a){
    //if builtin them simply apply that
    if(f->builtin){return f->builtin(e,a);}

    //Recore Argument count
    int given =a->count;
    int total=f->formals->count;

    //while arguments still remain to be processed
    while(a->count ){
        //if we have run out of formal arguments to bind
        if(f->formals->count==0){
            lval_del(a);
            return lval_err(
                "Function passed  in too many arguments ."
                "Got :%i , Expected : %i." ,given,total);
        }
        //pop the first symbol from the formals
        lval* sym =lval_pop(f->formals,0);

        //Special case to deal with '&'
        if(strcmp(sym->sym,"&")==0){
            //Ensure '&' is followed by another symbol
            if(f->formals->count!= 1){
                lval_del(a);
                return lval_err("Function format invalid."
                "Symbol '&' not followed by a single symbol");
            }
            //Next formal should be bound to remaining argument
            lval* nsym= lval_pop(f->formals,0);
            lenv_put(f->env,nsym,builtin_list(e,a));
            lval_del(sym);
            lval_del(nsym);
            break;

            
        }
        //pop the next argument from the list
        lval* val=lval_pop(a,0);
      
        //bind a copy into the function's enviornments
        lenv_put(f->env,sym,val);
        //delete symbol and values
        lval_del(sym);lval_del(val);
    }
    //Arguments list is now bound so can be cleaned up
    lval_del(a);

    //if '&' remains in formal list bind to empty list
    if(f->formals->count >0 && strcmp(f->formals->cell[0]->sym, "&")==0){
        // Check to ensure that & is not passed invalidly
        if(f->formals->count!= 2){
            return lval_err("Function formal invalid"
            "Symbol '&' not followed by single symbol");
        }
        // Pop and delete '&' symbol
        lval_del(lval_pop(f->formals,0));

        //pop next symbol and create empty list
        lval* sym=lval_pop(f->formals,0);
        lval* val=lval_qexpr();

        //Bind to enviorment and delete
        lenv_put(f->env,sym,val);
        lval_del(sym);lval_del(val);
    }


    // if all formals have been bound evaluate
    if(f->formals->count==0){
        //set enviornment parent to evaluation enviornment
        f->env->par=e;
        //EValuate and return
        return builtin_eval(f->env,
        lval_add(lval_sexpr(),lval_copy(f->body)));

    }
    else{
        //Otherwise return partially evaluated function
        return lval_copy(f);
    }
}


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
       lval* err =lval_err(
        "S-Expression starts with incorrect type"
        "Got : %s,Expected : %s",
        ltype_name (f->type),ltype_name(LVAL_FUN));
        lval_del(f);lval_del(v);
        return err;
    }
    lval* result =lval_call(e,f,v);
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
    symbol:  /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&|]+/ ;    \
    sexpr:'('<expr>* ')';                           \
    qexpr : '{' <expr>* '}';                        \
    expr: <number>| <symbol>|<sexpr>| <qexpr>;      \
    lispy: /^/ <expr>* /$/;                         \
    ",
    Number,Symbol,Sexpr,Qexpr,Expr,Lispy);

    puts("Lispy Version 0.0.0.0.8");
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
