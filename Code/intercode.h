#ifndef __INTERCODE_H__
#define __INTERCODE_H__

#include <stdio.h>
#include <assert.h>
#include "syntaxTree.h"
#include "symbol.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>


/* 操作数结构 */
typedef struct _Operand* Operand;
struct _Operand{
    enum OpKind{
        Em_VARIABLE,    // 变量（var）
        Em_CONSTANT,    // 常量（#1）
        Em_ADDRESS,     // 地址（&var）
        Em_LABEL,       // 标签(LABEL label1:)
        Em_ARR,         // 数组（arr[2]）
        //Em_STRUCT,      // 结构体（struct Point p）
        Em_TEMP         // 临时变量（t1）
    } kind;
    union{ 
        int varno;      // 变量定义的序号
        int labelno;    // 标签序号
        int val;        // 操作数的值
        int tempno;     // 临时变量序号（唯一性）
    } u;
    Type type;          // 计算数组、结构体占用size
    int para;           // 标识函数参数
    int isaddr;
};


/* 中间代码表示结构 */
typedef struct _InterCode* InterCode;;
struct _InterCode{
    enum ICKind {
        IC_ASSIGN,
        IC_LABEL,
        IC_PLUS,
        IC_MINUS,
        IC_MUL, 
        IC_DIV,
        //... 
        IC_FUNC, // ?? in FuncDec
        //
        IC_GET_ADDR, // x := &y
        IC_DEREF, // x := *y
        IC_STORE, // *x := y
        //
        IC_IFGOTO, // ?? in Cond
        IC_GOTO, // ?? in Cond
        //
        IC_CALL, 
        IC_PARAM, 
        IC_ARG, // 
        IC_READ, 
        IC_WRITE, 
        IC_RETURN,
        //...
        IC_DEC,
    } kind; 
    union{
        Operand op;
        char *func;
        struct{Operand right, left; } assign;     
        struct{Operand result, op1, op2; } binop; //三地址代码
        struct{Operand t1, t2; char relop[2 + 2]/*, ..*/; Operand label; } if_goto; // <============== Cond TODO!!
        // ..
        struct{Operand result; char *func;} call;
        struct{Operand op; int size;} arr;
    }u;
};


/* 中间代码列表的表示结构 */
typedef struct _CodeList* CodeList;
typedef struct _ArgList* ArgList;
typedef struct _Variable* Variable;
struct _CodeList{
    InterCode code;      // 中间代码列表实现方式
    CodeList prev, next;
};
struct _ArgList{         // 参数列表实现方式
    Operand args;
    ArgList next;
};
struct _Variable{       // 变量的实现方式
    char* name;
    Operand op;
    Variable next;
};
extern int var_num, label_num,temp_num; // 新变量/新标签/新临时变量编号
extern CodeList code_head, code_tail;  // 双链表的首部和尾部
extern Variable var_head,var_tail;     // 变量表的首部和尾部

CodeList getIntercode(node_t* root);
CodeList translate_ExtDef(node_t* ExtDef);
CodeList translate_FunDec(node_t* FunDec);
CodeList translate_CompSt(node_t* CompSt);
CodeList translate_DefList(node_t* DefList);
CodeList translate_Def(node_t* Def);
CodeList translate_StmtList(node_t* StmtList);
CodeList translate_Stmt(node_t* Stmt);
CodeList translate_Exp(node_t* Exp, Operand place);
CodeList translate_Args(node_t* Args, ArgList* argList);
CodeList translate_Cond(node_t *Exp, Operand label_true, Operand label_false);


Operand newOperand(enum OpKind kind, int value);
Operand new_var();
Operand new_arr();
Operand new_temp();
Operand new_constant(int value);
Operand new_label();
Operand lookup_var(char* name);
Operand deal_array(char* name, int* size);

InterCode new_InterCode(enum ICKind kind);
CodeList new_CodeList(InterCode ic);
CodeList create_code_Op(Operand op, enum ICKind kind);
CodeList concatenate(CodeList c1, CodeList c2);
CodeList CodePlus(int n, ...); // <==========================

void oprandToString(Operand op, char* buffer);
void get_IR(CodeList cur, FILE* ff);
void print_IR(CodeList codelisthead, FILE* ff);

#endif