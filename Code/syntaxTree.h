#ifndef __SYNTAX_TREE_H__
#define __SYNTAX_TREE_H__

#include <stdio.h>

typedef struct _node_t
{
    int lineno;
    char type_name[32];
	char id[32 + 2];
    int type;
    union {
        int type_int;
        float type_float;
    }num;
    struct _node_t *fir, *sib;
} node_t;

enum {
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_ID,
    TOKEN_TYPE,
    TOKEN_OTHER,
    TOKEN_SYNTAX,
};

node_t* create_node(int token_type, char *typename, int line, char *text);

void insert(node_t *par, node_t *son);

void print_tree(node_t *node, int nr_space);

#endif