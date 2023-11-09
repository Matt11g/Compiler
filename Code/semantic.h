#ifndef __SEMANTIC_H__
#define __SEMANTIC_H__

#include "syntaxTree.h"
#include "symbol.h"
#include <assert.h>
#include <time.h>

/* Entry */
void semantic_check(node_t* root);

/* Traverse global syntax tokens */
void ExtDefList(node_t* node);
void ExtDef(node_t* node);

void ExtSpecifier(node_t* node);
Type Specifier(node_t* node);

/* Traverse local blocks */
void CompSt(node_t* node, Type ntype);
void Stmt(node_t* node, Type ntype);

Type Exp(node_t* node); 

/* Fill in Symbol Table */
FieldList VarDec(node_t* node, Type type, int structflag);
Function FunDec(node_t* node, Type type);
FieldList VarList(node_t* node);
Type StructSpecifier(node_t* node);

#endif