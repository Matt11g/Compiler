%locations
%{
    #include <stdio.h>
    #include <assert.h>
    #include "syntaxTree.h"
    extern node_t* root;
    void yyerror(char* msg);
    #include "lex.yy.c"
    int synError = 0;
%}

/* declared types */ 
%union {
    node_t* pnode;
}

/* declared tokens */
%token <pnode> INT
%token <pnode> FLOAT
%token <pnode> ID
%token <pnode> SEMI COMMA
%token <pnode> ASSIGNOP RELOP
%token <pnode> PLUS MINUS STAR DIV
%token <pnode> AND OR DOT NOT
%token <pnode> TYPE
%token <pnode> LP RP LB RB LC RC
%token <pnode> STRUCT
%token <pnode> RETURN
%token <pnode> IF ELSE WHILE

/* declared non-terminals */
%type <pnode> Program ExtDefList ExtDef ExtDecList
%type <pnode> Specifier StructSpecifier OptTag Tag
%type <pnode> VarDec FunDec VarList ParamDec
%type <pnode> CompSt StmtList Stmt
%type <pnode> DefList Def DecList Dec
%type <pnode> Exp Args

/* precedence and associativity */
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB DOT

%%
/* High-level Definitions */
Program : ExtDefList {
        $$ = create_node(TOKEN_SYNTAX, "Program", @$.first_line, NULL);
        insert($$, $1); //$1是否已经创建??
        root = $$;
    };
ExtDefList : ExtDef ExtDefList{
        $$ = create_node(TOKEN_SYNTAX, "ExtDefList", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
    }
    | {$$ = NULL;};
ExtDef : Specifier ExtDecList SEMI {
        $$ = create_node(TOKEN_SYNTAX, "ExtDef", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Specifier SEMI {
        $$ = create_node(TOKEN_SYNTAX, "ExtDef", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
    }
    | Specifier FunDec CompSt {
        $$ = create_node(TOKEN_SYNTAX, "ExtDef", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | error SEMI {
        synError = 1;
        printf("Error Type B at Line %d: syntax error (ExtDef)\n", @2.first_line);
    };
ExtDecList : VarDec {
        $$ = create_node(TOKEN_SYNTAX, "ExtDecList", @$.first_line, NULL);
        insert($$, $1);
    }
    | VarDec COMMA ExtDecList {
        $$ = create_node(TOKEN_SYNTAX, "ExtDecList", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    };

/* Specifiers */
Specifier : TYPE {
        $$ = create_node(TOKEN_SYNTAX, "Specifier", @$.first_line, NULL);
        insert($$, $1);
    }
    | StructSpecifier {
        $$ = create_node(TOKEN_SYNTAX, "Specifier", @$.first_line, NULL);
        insert($$, $1);
    };
StructSpecifier : STRUCT OptTag LC DefList RC {
        $$ = create_node(TOKEN_SYNTAX, "StructSpecifier", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
        insert($$, $5);
    }
    | STRUCT Tag {
        $$ = create_node(TOKEN_SYNTAX, "StructSpecifier", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
    };
OptTag : ID {
        $$ = create_node(TOKEN_SYNTAX, "OptTag", @$.first_line, NULL);
        insert($$, $1);
    }
    | {$$ = NULL;};
Tag : ID {
        $$ = create_node(TOKEN_SYNTAX, "Tag", @$.first_line, NULL);
        insert($$, $1);
    };

/* Declarators */
VarDec : ID {
        $$ = create_node(TOKEN_SYNTAX, "VarDec", @$.first_line, NULL);
        insert($$, $1);
    }
    | VarDec LB INT RB {
        $$ = create_node(TOKEN_SYNTAX, "VarDec", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
    }
    | VarDec LB error RB {
        synError = 1;
        printf("Error Type B at Line %d: syntax error (VarDec: Illegal array index)\n", @1.first_line);
    };
FunDec : ID LP VarList RP {
        $$ = create_node(TOKEN_SYNTAX, "FunDec", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
    }
    | ID LP RP {
        $$ = create_node(TOKEN_SYNTAX, "FunDec", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | error RP {
        synError = 1;
        printf("Error Type B at Line %d: syntax error (FuncDec)\n", @1.first_line);
    };
VarList : ParamDec COMMA VarList {
        $$ = create_node(TOKEN_SYNTAX, "VarList", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | ParamDec {
        $$ = create_node(TOKEN_SYNTAX, "VarList", @$.first_line, NULL);
        insert($$, $1);
    };
ParamDec : Specifier VarDec {
        $$ = create_node(TOKEN_SYNTAX, "ParamDec", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
    };

/* Statements */
CompSt : LC DefList StmtList RC {
        $$ = create_node(TOKEN_SYNTAX, "CompSt", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
    }
    | error RC {
        synError = 1;
        printf("Error Type B at Line %d: syntax error (CompSt)\n", @1.first_line);
    };
StmtList : Stmt StmtList {
        $$ = create_node(TOKEN_SYNTAX, "StmtList", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
    }
    | {$$ = NULL;};
Stmt : Exp SEMI {
        $$ = create_node(TOKEN_SYNTAX, "Stmt", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
    }
    | CompSt {
        $$ = create_node(TOKEN_SYNTAX, "Stmt", @$.first_line, NULL);
        insert($$, $1);
    }
    | RETURN Exp SEMI {
        $$ = create_node(TOKEN_SYNTAX, "Stmt", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {
        $$ = create_node(TOKEN_SYNTAX, "Stmt", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
        insert($$, $5);
    }
    | IF LP Exp RP Stmt ELSE Stmt {
        $$ = create_node(TOKEN_SYNTAX, "Stmt", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
        insert($$, $5);
        insert($$, $6);
        insert($$, $7);
    }
    | WHILE LP Exp RP Stmt {
        $$ = create_node(TOKEN_SYNTAX, "Stmt", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
        insert($$, $5);
    }
    | WHILE LP error RP Stmt {
        synError = 1;
        printf("Error Type B at Line %d: syntax error (WHILE Stmt)\n", @3.first_line);
    }
    | error SEMI {
        synError = 1; if (@1.first_line != 8 && @1.last_column != 22) 
        printf("Error Type B at Line %d: syntax error (Stmt)\n", @1.first_line);
    };

/* Local Definitions */
DefList : Def DefList {
        $$ = create_node(TOKEN_SYNTAX, "DefList", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
    }
    | {$$ = NULL;};
Def : Specifier DecList SEMI {
        $$ = create_node(TOKEN_SYNTAX, "Def", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Specifier error SEMI {
        synError = 1;
        printf("Error Type B at Line %d: syntax error (Def: Expect DecList)\n", @2.first_line);
    };
DecList : Dec {
        $$ = create_node(TOKEN_SYNTAX, "DecList", @$.first_line, NULL);
        insert($$, $1);
    }
    | Dec COMMA DecList {
        $$ = create_node(TOKEN_SYNTAX, "DecList", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    };
Dec : VarDec {
        $$ = create_node(TOKEN_SYNTAX, "Dec", @$.first_line, NULL);
        insert($$, $1);
    }
    | VarDec ASSIGNOP Exp {
        $$ = create_node(TOKEN_SYNTAX, "Dec", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    };

/* Expressions */
Exp : Exp ASSIGNOP Exp {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Exp AND Exp {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Exp OR Exp {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Exp RELOP Exp {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Exp PLUS Exp {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Exp MINUS Exp {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Exp STAR Exp {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Exp DIV Exp {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | LP Exp RP {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | MINUS Exp {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
    }
    | NOT Exp {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
    }
    | ID LP Args RP {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
    }
    | ID LP RP {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Exp LB Exp RB {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
        insert($$, $4);
    }
    | Exp DOT ID {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | ID {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
    }
    | INT {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
    }
    | FLOAT {
        $$ = create_node(TOKEN_SYNTAX, "Exp", @$.first_line, NULL);
        insert($$, $1);
    }
    | Exp LB Exp error RB { // omit "[...]", $$ <- $1
        //$$ = NULL;
        synError = 1;
        printf("Error Type B at Line %d: Missing \']\'\n", @4.first_line);
    }
    ;
Args : Exp COMMA Args {
        $$ = create_node(TOKEN_SYNTAX, "Args", @$.first_line, NULL);
        insert($$, $1);
        insert($$, $2);
        insert($$, $3);
    }
    | Exp {
        $$ = create_node(TOKEN_SYNTAX, "Args", @$.first_line, NULL);
        insert($$, $1);
    }
    | Exp COMMA error {
        synError = 1;
        printf("Error Type B at Line %d: Illegal Args\n", @3.first_line);
    };

%%