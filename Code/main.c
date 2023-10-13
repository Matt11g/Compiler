#include <stdio.h>
//#include "syntax.tab.h"
#include "syntaxTree.h"
extern node_t* root;
//extern FILE* yyin;
extern void yyrestart(FILE* input_file);
extern int yyparse(void);
extern int lexError, synError;
int main(int argc, char** argv) {
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    if (lexError == 0 && synError == 0) print_tree(root, 0);
    // if (argc > 1) { 
    //     if (!(yyin = fopen(argv[1], "r"))) { 
    //         perror(argv[1]); 
    //         return 1; 
    //     } 
    // } 
    // while (yylex() != 0); 
    return 0; 
}

void yyerror(char* msg) { 
    //fprintf(stderr, "error: %s\n", msg); 
}