%{ 
    //#include <stdio.h>
    #include "syntax.tab.h"
    #include "syntaxTree.h"
    int yycolumn = 1;
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng;
    int show = 0;
    int lexError = 0;
%}
%option yylineno


delim  [ \t\r]
ws {delim}+

INT 0|[1-9][0-9]*
FLOAT [0-9]+\.[0-9]+
ID [_a-zA-Z][_a-zA-Z0-9]*
SEMI ";"
COMMA ","
ASSIGNOP "="
RELOP ">"|"<"|">="|"<="|"=="|"!="
PLUS "+"
MINUS "-"
STAR "*"
DIV "/"
AND "&&"
OR "||"
DOT "."
NOT "!"
TYPE int|float
LP "("
RP ")"
LB "["
RB "]"
LC "{"
RC "}"
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while

%%
{INT} { if (show) printf("INT\n"); yylval.pnode = create_node(TOKEN_INT, "INT", yylineno, yytext); return INT;}
{FLOAT} { if (show) printf("FLOAT\n"); yylval.pnode = create_node(TOKEN_FLOAT, "FLOAT", yylineno, yytext); return FLOAT;}
{SEMI} { if (show) printf("SEMI\n"); yylval.pnode = create_node(TOKEN_OTHER, "SEMI", yylineno, yytext); return SEMI;}
{COMMA} { if (show) printf("COMMA\n"); yylval.pnode = create_node(TOKEN_OTHER, "COMMA", yylineno, yytext); return COMMA;}
{ASSIGNOP} { if (show) printf("ASSIGNOP\n"); yylval.pnode = create_node(TOKEN_OTHER, "ASSIGNOP", yylineno, yytext); return ASSIGNOP;}
{RELOP} { if (show) printf("RELOP\n"); yylval.pnode = create_node(TOKEN_OTHER, "RELOP", yylineno, yytext); return RELOP;}
{PLUS} { if (show) printf("PLUS\n"); yylval.pnode = create_node(TOKEN_OTHER, "PLUS", yylineno, yytext); return PLUS;}
{MINUS} { if (show) printf("MINUS\n"); yylval.pnode = create_node(TOKEN_OTHER, "MINUS", yylineno, yytext); return MINUS;}
{STAR} { if (show) printf("STAR\n"); yylval.pnode = create_node(TOKEN_OTHER, "STAR", yylineno, yytext); return STAR;}
{DIV} { if (show) printf("DIV\n"); yylval.pnode = create_node(TOKEN_OTHER, "DIV", yylineno, yytext); return DIV;}
{AND} { if (show) printf("AND\n"); yylval.pnode = create_node(TOKEN_OTHER, "AND", yylineno, yytext); return AND;}
{OR} { if (show) printf("OR\n"); yylval.pnode = create_node(TOKEN_OTHER, "OR", yylineno, yytext); return OR;}
{DOT} { if (show) printf("DOT\n"); yylval.pnode = create_node(TOKEN_OTHER, "DOT", yylineno, yytext); return DOT;}
{NOT} { if (show) printf("NOT\n"); yylval.pnode = create_node(TOKEN_OTHER, "NOT", yylineno, yytext); return NOT;}
{TYPE} { if (show) printf("TYPE\n"); yylval.pnode = create_node(TOKEN_TYPE, "TYPE", yylineno, yytext); return TYPE;}
{LP} { if (show) printf("LP\n"); yylval.pnode = create_node(TOKEN_OTHER, "LP", yylineno, yytext); return LP;}
{RP} { if (show) printf("RP\n"); yylval.pnode = create_node(TOKEN_OTHER, "RP", yylineno, yytext); return RP;}
{LB} { if (show) printf("LB\n"); yylval.pnode = create_node(TOKEN_OTHER, "LB", yylineno, yytext); return LB;}
{RB} { if (show) printf("RB\n"); yylval.pnode = create_node(TOKEN_OTHER, "RB", yylineno, yytext); return RB;}
{LC} { if (show) printf("LC\n"); yylval.pnode = create_node(TOKEN_OTHER, "LC", yylineno, yytext); return LC;}
{RC} { if (show) printf("RC\n"); yylval.pnode = create_node(TOKEN_OTHER, "RC", yylineno, yytext); return RC;}
{STRUCT} { if (show) printf("STRUCT\n"); yylval.pnode = create_node(TOKEN_OTHER, "STRUCT", yylineno, yytext); return STRUCT;}
{RETURN} { if (show) printf("RETURN\n"); yylval.pnode = create_node(TOKEN_OTHER, "RETURN", yylineno, yytext); return RETURN;}
{IF} { if (show) printf("IF\n"); yylval.pnode = create_node(TOKEN_OTHER, "IF", yylineno, yytext); return IF;}
{ELSE} { if (show) printf("ELSE\n"); yylval.pnode = create_node(TOKEN_OTHER, "ELSE", yylineno, yytext); return ELSE;}
{WHILE} { if (show) printf("WHILE\n"); yylval.pnode = create_node(TOKEN_OTHER, "WHILE", yylineno, yytext); return WHILE;}

{ID} { if (show) printf("ID\n"); yylval.pnode = create_node(TOKEN_ID, "ID", yylineno, yytext); return ID;} //??

{ws} {}
\n { yycolumn = 1;}
"/*"(([^\*])|((\*)+[^\*/]))*(\*)+"/" {}
"//".* {}

[0-9]+[_a-zA-Z]+[0-9]* {lexError = 1; printf("Error type B at Line %d: Illegal identifier \'%s\'\n", yylineno, yytext); //无语??
        return ID;
    }
0[0-9]+ {lexError = 1; printf("Error type A at Line %d: Illegal INT number (Oct number not identified)\'%s\'\n", yylineno, yytext); return INT; }
([0-9]*\.[0-9]*[eE][-+]?[0-9]*)|([0-9]+\.)|(\.[0-9]+)|([0-9]*\.[0-9]*([0-9]*\.[0-9]*)+) {lexError = 1; printf("Error type A at Line %d: Illegal FLOAT number \'%s\'\n", yylineno, yytext); return FLOAT; }
.  {lexError = 1; printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext); 
     return SEMI; //导致syntax error，怎么解决
    }
%%