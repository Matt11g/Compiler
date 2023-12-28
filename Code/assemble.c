#include "assemble.h"
// Register r[32]; // 定义寄存器
// VarStructure varlist = NULL;
int argNum = 0;
char* reg_name[32] = {
    "zero", 
    "at", 
    "v0", "v1", 
    "a0", "a1", "a2", "a3", 
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", 
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", 
    "t8", "t9", 
    "k0", "k1", 
    "gp", 
    "sp", 
    "fp", 
    "ra", 
};

offsetTable* ptable = NULL;

int getOpIdx(Operand op) {
    assert(op);
    switch (op->kind) {
        case Em_TEMP: return var_num + op->u.tempno;
        case Em_VARIABLE: return op->u.varno;
        case Em_ADDRESS: assert(0);
        case Em_ARR: return op->u.varno;
        case Em_CONSTANT: return -1;
        case Em_LABEL: assert(0);
        default: assert(0);
    }
    return -1;
}

int getOffset(Operand op) {
    int idx = getOpIdx(op);
    assert(idx >= 0 && ptable[idx].alloc == 1);
    return ptable[idx].offset;
}

void loadToReg(Operand op, int reg, FILE* file) {
    switch (op->kind) {
        case Em_VARIABLE: 
        case Em_TEMP: 
            fprintf(file, "\tlw $%s, %d($fp)\n", reg_name[reg]/*寄存器x的名称*/, getOffset(op)); break;
        case Em_CONSTANT: fprintf(file, "\tli $%s, %d\n", reg_name[reg]/*寄存器x的名称*/, op->u.val/*常量值*/); break;
        default: assert(0);
    }
}

void saveFromReg(Operand op, int reg, FILE* file) {
    switch (op->kind) {
        case Em_VARIABLE: 
        case Em_TEMP: 
            fprintf(file, "\tsw $%s, %d($fp)\n", reg_name[reg]/*寄存器x的名称*/, getOffset(op)); break;
        case Em_ARR://MODIFIED
            fprintf(file, "\tsw $%s, %d($fp)\n", reg_name[reg]/*寄存器x的名称*/, getOffset(op)); break;
        default: assert(0);
    }
}

//记录堆栈偏移位置、寄存器及函数实参或形参数目;
void generate_asm(CodeList codehead, FILE* file) {
    // 打印目标代码数据段
    fprintf(file, ".data\n_prompt: .asciiz \"Enter an integer:\"\n_ret: .asciiz \"\\n\"\n.globl main\n.text\n");
    // 打印read函数定义
    fprintf(file, "\nread:\n");
    fprintf(file, "\tli $v0, 4\n");
    fprintf(file, "\tla $a0, _prompt\n");
    fprintf(file, "\tsyscall\n");
    fprintf(file, "\tli $v0, 5\n");
    fprintf(file, "\tsyscall\n");
    fprintf(file, "\tjr $ra\n");
    // 打印write函数定义
    fprintf(file,"\nwrite:\n");
    fprintf(file,"\tli $v0, 1\n");
    fprintf(file,"\tsyscall\n");
    fprintf(file,"\tli $v0, 4\n");
    fprintf(file,"\tla $a0, _ret\n");
    fprintf(file,"\tsyscall\n");
    fprintf(file,"\tmove $v0, $0\n");
    fprintf(file,"\tjr $ra\n\n");
    
    // for(int i = 0; i < 32; i++){
    //     Register x = (Register)malloc(sizeof(struct _Register));
    //     strcpy(x->name, reg_name[i]);
    //     x->var = NULL;  // 设置寄存器关联的变量，初始可以置为空;
    //     r[i] = x;
    // }
    ptable = (offsetTable *)malloc(sizeof(offsetTable) * (var_num + temp_num + 8));
    memset(ptable, 0, sizeof(offsetTable) * (var_num + temp_num + 8));
    CodeList curcode = codehead;
    do {
        if (curcode->code->kind == IC_FUNC) {
            fprintf(file, "\n%s:\n", curcode->code->u.func);//向目标代码文件中写入文件名;
            fprintf(file, "\tsubu $sp, $sp, 4\n"); // 开辟栈空间
            fprintf(file, "\tsw $ra, 0($sp)\n"); // 保存$ra(返回地址)
            fprintf(file, "\tsubu $sp, $sp, 4\n"); // 开辟栈空间
            fprintf(file, "\tsw $fp, 0($sp)\n"); // 保存$fp
            fprintf(file, "\tmove $fp, $sp\n"); // 重新设置$fp
            // 申请一段较大的栈空间（可通过$fp加上一段偏移寻址），保存局部变量等;
            // 初始化堆栈偏移量，函数形参个数等;
            int stackSize = allocStack(curcode);
            assert(stackSize * 4 < (1 << 15));
            fprintf(file, "\taddi $sp, $sp, -%d\n", stackSize * 4); // 开辟栈空间
        }
        else generate_IR_asm(curcode->code, file); // 遍历每一条IR，生成目标代码
        curcode = curcode->next;//获取下一条IR;
    }while(curcode != codehead);
    free(ptable);
}

void generate_IR_asm(InterCode intercode, FILE* file) {
    //printf("kind == %d\n", intercode->kind);
    switch(intercode->kind){
        case IC_LABEL:
            fprintf(file, "label%d:\n", intercode->u.op->u.labelno);//向目标代码文件中写入label名;
            break;
        case IC_FUNC:
            break;
        case IC_ASSIGN:{
            if (intercode->u.assign.left == NULL) break; 
            Operand left = intercode->u.assign.left, right = intercode->u.assign.right;
            
            loadToReg(right, t1, file);

            fprintf(file, "\tmove $t0, $t1\n");

            saveFromReg(left, t0, file);
            break;
        }
        case IC_GET_ADDR: {
            if (intercode->u.assign.left == NULL) break; 
            Operand left = intercode->u.assign.left, right = intercode->u.assign.right;
            //loadToReg(right, t1, file);
            fprintf(file, "\taddi $t0, $fp, %d\n", getOffset(right));  
            saveFromReg(left, t0, file);
            break;
        }
        case IC_DEREF: {
            if (intercode->u.assign.left == NULL) break; 
            loadToReg(intercode->u.assign.right, t1, file);
            fprintf(file, "\tlw $t0, 0($t1)\n");
            saveFromReg(intercode->u.assign.left, t0, file);
            break;
        }
        case IC_STORE: {
            if (intercode->u.assign.left == NULL) break; 
            loadToReg(intercode->u.assign.right, t1, file);
            loadToReg(intercode->u.assign.left, t0, file);
            fprintf(file, "\tsw $t1, 0($t0)\n");
            break;
        }
        case IC_PLUS:
        case IC_MINUS:
        case IC_MUL:
        case IC_DIV: {
            Operand result = intercode->u.binop.result, lhs = intercode->u.binop.op1, rhs = intercode->u.binop.op2; //获取三个操作数result, lhs, rhs;
            loadToReg(lhs, t1, file);//为其分配寄存器x, y, z，如果左右操作数是常数，将其加载到寄存器中;
            loadToReg(rhs, t2, file);
            switch (intercode->kind) {
                case IC_PLUS:
                    fprintf(file, "\tadd $t0, $t1, $t2\n"); break;
                case IC_MINUS:
                    fprintf(file, "\tsub $t0, $t1, $t2\n"); break;
                case IC_MUL:
                    fprintf(file, "\tmul $t0, $t1, $t2\n"); break;
                case IC_DIV:
                    fprintf(file, "\tdiv $t0, $t1, $t2\n");
                    fprintf(file, "\tmflo $t0\n");
                    break;
                default: assert(0);
            }
            saveFromReg(result, t0, file);//将寄存器x关联的变量值溢出到栈中记录偏移量，并退出;
            break;
        }
        case IC_IFGOTO:{
            char op[4];
            strcpy(op, intercode->u.if_goto.relop);
            loadToReg(intercode->u.if_goto.t1, t1, file);//为其分配寄存器x, y, z，如果左右操作数是常数，将其加载到寄存器中;
            loadToReg(intercode->u.if_goto.t2, t2, file);// 为其分配寄存器x, y，如果左右操作数是常数，将其加载到寄存器中;
            if(strcmp(op, "==") == 0)
                fprintf(file, "\tbeq $t1, $t2, label%d\n", intercode->u.if_goto.label->u.labelno);//"label%d", op->u.labelno
            else if(strcmp(op, "!=") == 0)
                fprintf(file, "\tbne $t1, $t2, label%d\n", intercode->u.if_goto.label->u.labelno);
            else if(strcmp(op, ">") == 0)
                fprintf(file, "\tbgt $t1, $t2, label%d\n", intercode->u.if_goto.label->u.labelno);
            else if(strcmp(op, "<") == 0)
                fprintf(file, "\tblt $t1, $t2, label%d\n", intercode->u.if_goto.label->u.labelno);
            else if(strcmp(op, ">=") == 0) // "<="与其类似
                fprintf(file, "\tbge $t1, $t2, label%d\n", intercode->u.if_goto.label->u.labelno);
            else if(strcmp(op, "<=") == 0) // "<="与其类似
                fprintf(file, "\tble $t1, $t2, label%d\n", intercode->u.if_goto.label->u.labelno);
            else assert(0);
            break;
        }
        case IC_ARG:{
            /*//找到参数对应的变量arg;
            fprintf(file, "\tlw $s0, %d($fp)\n", 2);//TODO: arg在栈中相对于$fp的偏移量
            fprintf(file, "\tsubu $sp, $sp, 4\n");
            fprintf(file, "\tsw $s0, 0($sp)\n");
            argNum++;*/

            loadToReg(intercode->u.op, a0, file);
            fprintf(file, "\taddi $sp, $sp, -4\n");
            fprintf(file, "\tsw $a0, 0($sp)\n");
            break;
        }
        case IC_CALL:{
            /*fprintf(file, "\tli $v1, %d\n", argNum * 4); // 实参占用的栈空间
            fprintf(file, "\tsubu $sp, $sp, 4\n");
            fprintf(file, "\tsw $v1, 0($sp)\n"); // 将实参占用栈空间保存在栈上
            argNum = 0; // 调用前处理ARG空间
            fprintf(file,"\tsubu $sp, $sp, 4\n");
            fprintf(file,"\tsw $ra, 0($sp)\n"); // 保存$ra寄存器
            //T 为返回值分配一个寄存器x;
            fprintf(file, "\tjal %s\n", intercode->u.call.func);
            fprintf(file, "\tmove %s, $v0\n", " name"); // 将x溢出到栈上;
            fprintf(file,"\tlw $ra, 0($sp)\n"); // 恢复$ra的值
            fprintf(file,"\taddi $sp, $sp, 4\n");
            //回收实参占用的栈空间并退出;*/

            fprintf(file, "\tjal %s\n", intercode->u.call.func);
            saveFromReg(intercode->u.op, v0, file);
            break;
        }
        case IC_READ:{
            /*fprintf(file, "\taddi $sp, $sp, -4\n");
            fprintf(file, "\tsw $ra, 0($sp)\n"); // 保存$ra寄存器内容
            fprintf(file, "\tjal read\n");
            //为read参数x分配寄存器;
            fprintf(file, "\tmove %s, $v0\n", "name");
            //将x关联到的变量值溢出到栈上并标记偏移量;
            fprintf(file, "\tlw $ra, 0($sp)\n");
            fprintf(file, "\taddi $sp, $sp, 4\n"); // 恢复$ra寄存器*/
            fprintf(file, "\tjal read\n");
            saveFromReg(intercode->u.op, v0, file);
            break;
        }
        case IC_WRITE:{
            /*fprintf(file, "\taddi $sp, $sp, -4\n");
            fprintf(file, "\tsw $ra, 0($sp)\n"); // 保存$ra寄存器内容
            //为x分配寄存器;
            if(intercode->u.op->kind == Em_VARIABLE || intercode->u.op->kind == Em_TEMP){
                fprintf(file,"\tmove $a0, %s\n", " name");
            }else if(intercode->u.op->kind == Em_ADDRESS || intercode->u.op->kind == Em_ARR){
                fprintf(file, "\tlw $a0, 0(%s)\n", " 变量或者数组名");
            }
            fprintf(file,"\tjal write\n"); // 跳转到write函数
            // 将寄存器x关联的变量值溢出到栈中记录偏移量;
            fprintf(file,"\tlw $ra, 0($sp)\n");
            fprintf(file,"\taddi $sp, $sp, 4\n"); // 恢复$ra内容*/
            loadToReg(intercode->u.op, a0, file);
            fprintf(file,"\tjal write\n"); // 跳转到write函数
            break;
        }
        case IC_GOTO: {
            fprintf(file, "\tj label%d\n", intercode->u.op->u.labelno);
            break;
        }
        case IC_RETURN: {
            loadToReg(intercode->u.op, v0, file);
            fprintf(file, "\tmove $sp, $fp\n");//

            fprintf(file, "\tlw $fp, 0($sp)\n");
            fprintf(file, "\taddi $sp, $sp, 4\n");
            fprintf(file, "\tlw $ra, 0($sp)\n");
            fprintf(file, "\taddi $sp, $sp, 4\n");

            fprintf(file, "\tjr $ra\n");
            break;
        }
        case IC_PARAM:
        case IC_DEC: 
            break;
        default: assert(0); 
    }
}

// int getReg(Operand op) {
//      // 获取寄存器并保存常量
// }

int allocStack(CodeList entry) {
    CodeList cur = entry->next;
    int paramCnt = 0;
    int byte4Cnt = 0;
    int exit__ = 0;
    do {
        assert(cur);
        switch (cur->code->kind) {
            case IC_PARAM:{
                int idx = getOpIdx(cur->code->u.op);
                if (ptable[idx].alloc == 0) {
                    ptable[idx].alloc = 1;
                    paramCnt++;
                    ptable[idx].offset = 4 * (paramCnt + 1);
                }
                break;
            }
            case IC_ARG:
            case IC_RETURN:
            case IC_READ:
            case IC_WRITE:{
                int idx = getOpIdx(cur->code->u.op);
                if (idx >= 0 && ptable[idx].alloc == 0){
                    ptable[idx].alloc = 1;
                    byte4Cnt++;
                    ptable[idx].offset = -4 * byte4Cnt;
                }
                break;
            }
            case IC_CALL:{
                if (cur->code->u.call.result == NULL) assert(0);
                int idx = getOpIdx(cur->code->u.call.result);
                if (idx >= 0 && ptable[idx].alloc == 0){
                    ptable[idx].alloc = 1;
                    byte4Cnt++;
                    ptable[idx].offset = -4 * byte4Cnt;
                }
                break;
            }
            case IC_DEC:{
                int idx = getOpIdx(cur->code->u.arr.op);
                if (idx >= 0 && ptable[idx].alloc == 0){
                    ptable[idx].alloc = 1;
                    byte4Cnt += cur->code->u.arr.size / 4;
                    ptable[idx].offset = -4 * byte4Cnt;
                }
                break;
            }
            case IC_IFGOTO:{
                int idx[2] = {getOpIdx(cur->code->u.if_goto.t1), getOpIdx(cur->code->u.if_goto.t2)};
                for (int i = 0; i < 2; i++) {
                    if (idx[i] >= 0 && ptable[idx[i]].alloc == 0) {
                        ptable[idx[i]].alloc = 1;
                        byte4Cnt++;
                        ptable[idx[i]].offset = -4 * byte4Cnt;
                    }
                }
                break;
            }
            case IC_ASSIGN:
            case IC_GET_ADDR: 
            case IC_DEREF: 
            case IC_STORE: {
                if (cur->code->u.assign.left == NULL) break; 
                int idx[2];
                idx[0] = getOpIdx(cur->code->u.assign.left);
                idx[1] = getOpIdx(cur->code->u.assign.right);
                for (int i = 0; i < 2; i++) {
                    if (idx[i] >= 0 && ptable[idx[i]].alloc == 0) {
                        ptable[idx[i]].alloc = 1;
                        byte4Cnt++;
                        ptable[idx[i]].offset = -4 * byte4Cnt;
                    }
                }
                break;
            }
            case IC_PLUS:
            case IC_MINUS:
            case IC_MUL:
            case IC_DIV: {
                int idx[3] = {getOpIdx(cur->code->u.binop.result), getOpIdx(cur->code->u.binop.op1), getOpIdx(cur->code->u.binop.op2)};
                for (int i = 0; i < 3; i++) {
                    if (idx[i] >= 0 && ptable[idx[i]].alloc == 0) {
                        ptable[idx[i]].alloc = 1;
                        byte4Cnt++;
                        ptable[idx[i]].offset = -4 * byte4Cnt;
                    }
                }
                break;
            }
            case IC_FUNC: exit__ = 1; break;
            case IC_GOTO: break;
            case IC_LABEL: break;
            default: assert(0);
        }
        if (exit__)break;
        cur = cur->next;
    } while (cur != entry);
    return byte4Cnt;
}