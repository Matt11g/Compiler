#ifndef __ASSEMBLE_H__
#define __ASSEMBLE_H__

#include "intercode.h"

typedef struct _offsetTable {
    int alloc;
    int offset;
} offsetTable; 
extern char* reg_name[32];


enum REGS_NO {
    zero, 
    at, 
    v0, v1, 
    a0, a1, a2, a3, 
    t0, t1, t2, t3, t4, t5, t6, t7, 
    s0, s1, s2, s3, s4, s5, s6, s7, 
    t8, t9, 
    k0, k1, 
    gp, 
    sp, 
    fp, 
    ra, 
};

// struct _VarStructure{
//     // 变量名;
//     // 变量存放的寄存器;
//     // 变量在内存中的存储位置;
//     // 变量链表;
// };
// typedef struct _VarStructure* VarStructure;
// struct _Register{
//     char name[4];
//     VarStructure var;
// };
// typedef struct _Register* Register;

void generate_asm(CodeList codehead, FILE* file);
void generate_IR_asm(InterCode intercode, FILE* file);

int allocStack(CodeList entry);
#endif