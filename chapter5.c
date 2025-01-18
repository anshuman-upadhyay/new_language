//Parsing : allows raw user input or text to be converted into a structured format that the program can process like an abstract syntax tree(AST)
// #include "D:/CODING/self_projects/new_language/mpc/mpc.h" 
//* the above header file it the absolute path to access the mpc.h and mpc.c header file 
//~ #include "mpc/mpc.h"//* this on the hand extracts the content from the child mpc folder directly

// ~int main(int argc ,char** argv){
//~     //Build a parser 'Adjective' to recognize description
//~     mpc_parser_t* Adjective = mpc_or(4,
// ~    mpc_sym("wow"),mpc_sym("many"),
// ~    mpc_sym("so"),mpc_sym("such")
// ~    );
// ~    //Build a parser "Noun" to recognize things
// ~    mpc_parser_t* Noun=mpc_or(5,
// ~    mpc_sym("lisp"),mpc_sym("language"),mpc_sym("book"),mpc_sym("build"),mpc_sym("c")
// ~    );
// ~    mpc_parser_t* Phrase= mpc_and(2,mpc_strfold,Adjective,Noun,free);
// ~    mpc_parser_t* Doge =mpc_many(mpcf_strfold,Phrase);

//~     mpc_delete(Doge);
//~     return 0;
// ~}

//same code in a better wauy



#include "mpc/mpc.h"
int main(int argc,char**argv){
    mpc_parser_t* adjective=mpc_new("adjective");
    mpc_parser_t*noun=mpc_new("noun");
    mpc_parser_t*phrase=mpc_new("phrase");
    mpc_parser_t*doge=mpc_new("doge");

    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                   \
    adjective:\"wow\"|\"many\"|\"so\"|\"such\";          \
    noun:\"lisp\"|\"language\"|\"book\"|\"build\"|\"c\"; \
    phrase:<adjective><noun>;                            \
    doge:<phrase>*;                                      \
    ",
    adjective,noun,phrase,doge);
    mpc_cleanup(4,adjective,noun,phrase,doge);
    return 0;
}



// for compiling the code use : cc -std=c99 -Wall <filename>.c mpc.c -o parsing

