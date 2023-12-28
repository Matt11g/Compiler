#include <stdio.h>
//#include "syntax.tab.h"
#include "syntaxTree.h"
#include "semantic.h"
#include "intercode.h"
#include "assemble.h"
extern node_t* root;
//extern FILE* yyin;
extern void yyrestart(FILE* input_file);
extern int yyparse(void);
extern int lexError, synError;

//#define PRINT_TREE 1

int main(int argc, char** argv) {
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    // if (argc > 1) { 
    //     if (!(yyin = fopen(argv[1], "r"))) { 
    //         perror(argv[1]); 
    //         return 1; 
    //     } 
    // } 
    // while (yylex() != 0); 

#ifdef PRINT_TREE
    if (lexError == 0 && synError == 0) print_tree(root, 0);
#endif
    assert (lexError == 0 && synError == 0);
    if (lexError == 0 && synError == 0) { // 语法树正确构建
        semantic_check(root); // 语义分析
    }

//     FILE* ff = NULL;
// //中间代码生成
    CodeList codelisthead = getIntercode(root);
    // if (argv[2] == NULL) {ff = fopen("output", "w");}
    // else {
    //     ff = fopen(argv[2], "w");
    // }
    // print_IR(codelisthead, ff);
    // fclose(ff);

    FILE* asmff;
    if (argv[2] == NULL)asmff = fopen("output.s", "w");
    else asmff = fopen(argv[2], "w");
// 目标代码生成
    generate_asm(codelisthead, asmff);
    fclose(asmff);

    return 0;  
}

void yyerror(char* msg) { 
    //fprintf(stderr, "error: %s\n", msg); 
}