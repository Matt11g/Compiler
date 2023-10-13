#include "syntaxTree.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

node_t* root = NULL;

// typedef struct _node_t
// {
//  int lineno;
//  char type_name[32]; 
// 	char id[32 + 2];
//     int type;
//     union {
//         int type_int;
//         float type_float;
//     }num;
//     struct _node_t *fir, *sib;
// } node_t;

// enum {
//     TOKEN_INT,
//     TOKEN_FLOAT,
//     TOKEN_ID,
//     TOKEN_TYPE,
//     TOKEN_OTHER,
//     TOKEN_SYNTAX,
// };

node_t* create_node(int token_type, char *typename, int line, char *text) {
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = token_type;
    strcpy(node->type_name, typename);
    node->lineno = line;
    switch (token_type) {
        case TOKEN_ID: case TOKEN_TYPE: strcpy(node->id, text); break;
        case TOKEN_INT: node->num.type_int = atoi(text); break;
        case TOKEN_FLOAT: node->num.type_float = atof(text); break;
        default: break;
    }
    node->fir = NULL; node->sib = NULL;
    return node;
}

void insert(node_t *par, node_t *son) {
    assert(par);
    //assert(par && son); // No need!!
    if (par->fir) {
        node_t *p = par->fir;
        while (p->sib) p = p->sib;
        p->sib = son;
    }
    else {
        par->fir = son;
    }
}

void print_tree(node_t *node, int nr_space) {
    if (node) {
        for (int i = 0; i < nr_space; i++) printf(" ");
        // print_node
        switch (node->type) {
            case TOKEN_INT: printf("%s: %d\n", node->type_name, node->num.type_int); break;
            case TOKEN_FLOAT: printf("%s: %f\n", node->type_name, node->num.type_float); break;
            case TOKEN_ID: printf("%s: %s\n", node->type_name, node->id); break;
            case TOKEN_TYPE: printf("%s: %s\n", node->type_name, node->id); break;
            case TOKEN_OTHER: printf("%s\n", node->type_name); break;
            case TOKEN_SYNTAX: printf("%s (%d)\n", node->type_name, node->lineno); break;
            default: break;
        }
        //
        print_tree(node->fir, nr_space + 2);
        print_tree(node->sib, nr_space);
    }
}
