%{ 
    #include <stdio.h>
    #include "syntax.tab.h"
    int yycolumn = 1;
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng;
    int show = 0;
%}
%option yylineno


delim  [ \t\r]
ws {delim}+

INT [-+]?([0-9]|[1-9][0-9]*|0[0-7]*|0[xX][0-9a-fA-F]*)
FLOAT [-+]?[0-9]*\.[0-9]+([eE][-+]?[0-9]+)?
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
{INT} { if (show) printf("INT\n"); yylval.type_int = atoi(yytext); return INT;}
{FLOAT} { if (show) printf("FLOAT\n"); yylval.type_float = atof(yytext); return FLOAT;}
{SEMI} { if (show) printf("SEMI\n"); return SEMI;}
{COMMA} { if (show) printf("COMMA\n"); return COMMA;}
{ASSIGNOP} { if (show) printf("ASSIGNOP\n"); return ASSIGNOP;}
{RELOP} { if (show) printf("RELOP\n"); return RELOP;}
{PLUS} { if (show) printf("PLUS\n"); return PLUS;}
{MINUS} { if (show) printf("MINUS\n"); return MINUS;}
{STAR} { if (show) printf("STAR\n"); return STAR;}
{DIV} { if (show) printf("DIV\n"); return DIV;}
{AND} { if (show) printf("AND\n"); return AND;}
{OR} { if (show) printf("OR\n"); return OR;}
{DOT} { if (show) printf("DOT\n"); return DOT;}
{NOT} { if (show) printf("NOT\n"); return NOT;}
{TYPE} { if (show) printf("TYPE\n"); return TYPE;}
{LP} { if (show) printf("LP\n"); return LP;}
{RP} { if (show) printf("RP\n"); return RP;}
{LB} { if (show) printf("LB\n"); return LB;}
{RB} { if (show) printf("RB\n");return RB;}
{LC} { if (show) printf("LC\n");return LC;}
{RC} { if (show) printf("RC\n"); return RC;}
{STRUCT} { if (show) printf("STRUCT\n"); return STRUCT;}
{RETURN} { if (show) printf("RETURN\n"); return RETURN;}
{IF} { if (show) printf("IF\n"); return IF;}
{ELSE} { if (show) printf("ELSE\n"); return ELSE;}
{WHILE} { if (show) printf("WHILE\n"); return WHILE;}

{ID} { if (show) printf("ID\n"); return ID;} //??

{ws} {}
\n { yycolumn = 1;} //??, order??

.  {printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext); }
%%