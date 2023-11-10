#ifndef __SEMANTIC_H__
#define __SEMANTIC_H__

#include "syntaxTree.h"
#include "symbol.h"
#include <assert.h>
#include <time.h>

/* Entry */
void semantic_check(node_t* root);

void ExtDefList(node_t* node);
void ExtDef(node_t* node);
FieldList ExtDecList(node_t* node, Type type, int structflag);

void ExtSpecifier(node_t* node);
Type Specifier(node_t* node);
void ExtStructSpecifier(node_t* node);
Type StructSpecifier(node_t* node);

FieldList Def(node_t* node, int structflag);
FieldList DefList(node_t* node, int structflag);

void CompSt(node_t* node, Type ntype);
void StmtList(node_t* node, Type ntype);
void Stmt(node_t* node, Type ntype);

Type Exp(node_t* node); 
FieldList Args(node_t* node);   

FieldList VarDec(node_t* node, Type type, int structflag);
FieldList Dec(node_t* node, Type type, int structflag);
FieldList DecList(node_t* node, Type type, int structflag);

Function FunDec(node_t* node, Type type);
FieldList VarList(node_t* node);
FieldList ParamDec(node_t* node);

#endif