#include "intercode.h"

CodeList getIntercode(node_t* root){
    // 首先判断AST是否为空;
    // 接着判断AST结构是否正确（满足C—文法要求）;
    if (NULL == root) return NULL;
    if(strcmp(root->type_name, "Program")!= 0 ) return NULL;
    // 初始化全局变量中间代码变量等列表;
    code_head = code_tail = NULL;
    var_tail = var_head = NULL;
    // 设定变量、标签、临时变量标号;
    var_num = 1; label_num = 1; temp_num = 1;
    // 遍历AST的节点，依次调用相应的translate_X函数;
    node_t* node = root -> fir;  // ExtDefList
    while(node!= NULL){
        node = node ->fir;
        insert_code(translate_ExtDef(node));
        //递归遍历兄弟节点;
        node = node->sib; 
    }
    return code_head;
}

CodeList translate_ExtDef(node_t* ExtDef){
    // 检查语法树是否为空;
    // if (ExtDef == NULL) return NULL; ?? 
    // 检查是否为"；";
    if(strcmp(ExtDef->fir->sib->type_name, "SEMI") == 0) return NULL;
    // 检查是否有全局标识符定义;
    if(strcmp(ExtDef->fir->sib->type_name, "ExtDecList") == 0) return NULL;
    // 依次检查是否是函数定义;
    // 实验三不包含全局变量，结构体定义也已经在实验二中处理并添加到符号表中了
    if(strcmp(ExtDef->fir->sib->type_name, "FunDec") == 0){ // checkp(ExtDef, 3, "Specifier", "FunDec", "CompSt")  ??
        CodeList c1 = translate_FunDec(ExtDef->fir->sib);
        CodeList c2 = translate_CompSt(ExtDef->fir->sib->sib);
        // 合并函数声明和函数体
        return concatenate(c1, c2);
    } else{
        fprintf(stderr, "error ExtDef!\n"); return NULL;
    }
}

CodeList translate_FunDec(node_t* FunDec) {
    if(strcmp(FunDec->sib->sib->type_name, "RP") == 0){// 处理无参函数, checkp(FunDec, 3, "ID", "LP", "RP")
        // 初始化函数信息，获取函数名
        InterCode ic = new_InterCode(IC_FUNC);
        ic->u.func = FunDec->fir->id;
        CodeList c1 = new_CodeList(ic);
        return c1;
    } 
    else if(strcmp(FunDec->sib->sib->type_name, "VarList") == 0){// 处理有参函数, checkp(FunDec, 4, "ID", "LP", "VarList", "RP")
        // 初始化函数信息，获取函数名
        InterCode ic = new_InterCode(IR_FUNC);
        ic->u.func = FunDec->fir->id;
        CodeList c1 = new_CodeList(ic);
        // 获取参数列表
        FieldList params = Type_get(FunDec->fir->id)->u.function->param;
        while (params!=NULL)
        {
            // 构造函数参数列表判断参数类型（选做内容中需要支持结构体和数组作为参数）
            if(params->type->kind == BASIC) { 
                //...
            }
            else { // 处理数组或者结构体
            
            }
            //params = params->tail;// 遍历下一个？？
        } 
    }
    else { fprintf(stderr, "error FunDec!\n"); return NULL;}
}

CodeList translate_CompSt(node_t* CompSt) {
    if(CompSt == NULL) return NULL;
    assert(NULL != CompSt->fir);
    if(strcmp(CompSt->fir->sib->type_name, "RC") == 0) {
        //deleteScope();//<--------------------------------------
        return NULL;
    }
    // 处理DefList节点
    CodeList c1 = NULL;
    // 处理StmtList节点
    CodeList c2 = NULL;
    if(strcmp(CompSt->fir->sib->type_name, "DefList") == 0) {
        c1 = translate_DefList(CompSt->fir->sib);
        if (strcmp(CompSt->fir->sib->sib->type_name, "StmtList") == 0) c2 = translate_StmtList(CompSt->fir->sib->sib);
        //deleteScope();//<--------------------------------------
    }
    else {
        c1 = translate_StmtList(CompSt->fir->sib);
        //deleteScope();//<--------------------------------------
    }
    // 合并c1和c2
    return concatenate(c1, c2);
}

CodeList translate_Stmt(node_t* Stmt) {
    if (strcmp(Stmt->fir->type_name, "CompSt") == 0){ // 处理复合语句
        return translate_CompSt(Stmt->fir);
    } else if (strcmp(Stmt->fir->type_name, "Exp") == 0){ // 处理表达式
        return translate_Exp(Stmt->fir, NULL);
    } else if (strcmp(Stmt->fir->type_name, "RETURN") == 0){ // 处理返回语句
        Operand t1 = new_temp();
        CodeList c1 = translate_Exp(Stmt->fir->sib,t1);
        CodeList c2 = create_code_Op(t1, IC_RETURN);
        return concatenate(c1,c2);
    } else if (strcmp(Stmt->fir->type_name, "IF") == 0) { //  IF ELSE
        if (Stmt->fir->sib->sib->sib->sib->sib == NULL) { // true分支
            //...
        } else { // true/false分支
            //...
        }
    } else if (strcmp(Stmt->fir->type_name, "WHILE") == 0) { // while语句处理
        //...
    }
    return NULL;
}

CodeList translate_Exp(node_t* Exp, Operand place) { // place表示值
    if(strcmp(Exp->fir->type_name, "INT") == 0) { //INT
        int val = Exp->fir->num.type_int;
        InterCode ic = new_InterCode(IC_ASSIGN);
        ic->u.assign.left = place; // 构建左值
        ic->u.assign.right = new_constant(val); // 构建常量
        return new_CodeList(ic);
    }
    if(strcmp(Exp->fir->type_name,"ID") == 0){
        if(Exp->fir->sib == NULL){ // 处理标识符
            Operand op = lookup_var(Exp->fir->id); // 查找标识符
            if(op->kind == Em_ARR || op->kind == Em_STRUCT){ // 数组or结构体返回地址
                InterCode ic = new_InterCode(IC_GET_ADDR);
                ic->u.assign.left = place;
                ic->u.assign.right = op;
                return new_CodeList(ic);
            }
            else { // 一般变量直接赋值
            }
        }
        else if (strcmp(Exp->fir->sib->type_name, "LP") == 0) { // 函数调用
            if (strcmp(Exp->fir->sib->sib->type_name,"Args") == 0){ 
                Function fun = Type_get(Exp->fir->id)->u.function;
                // 获取函数参数列表并处理实参
                ArgList argList = NULL; // <======================================= ?? 
                CodeList c1 = translate_Args(Exp->fir->sib->sib, &argList);
                // 是否是write函数
                if(strcmp(fun->name, "write") == 0) {
                    InterCode ic = new_InterCode(IC_WRITE);
                    ic->u.op = argList->args;
                    return concatenate(c1, new_CodeList(ic));
                } else { // 用户自定义函数
                    CodeList c2 = NULL;
                    while (argList != NULL) {
                        //创建实参压栈IR;
                    }
                    //创建函数调用IR;
                    //合并操作;
                }
            }
            else {

            }
        }
        /*else if(strcmp(Exp->fir->sib->type_name, "DOT") == 0) { // 结构体访问
            // 构建临时变量
            Operand baseAddr = new_temp();
            baseAddr->kind = Em_ADDRESS;
            CodeList c1 = translate_Exp(Exp->fir, baseAddr);  // 获取结构体的地址
            InterCode ic = new_InterCode(IR_PLUS);
            Operand tmp = new_temp();
            tmp->kind = Em_ADDRESS;
            ic->u.binop.result = tmp;
            ic->u.binop.op1 = baseAddr;
            // 获取结构体域的偏移量
            ic->u.binop.op2 = size_get_instruct(Exp->fir->sib->sib->Valstr);
            CodeList c2 = new_CodeList(ic);
            InterCode ic2 = new_InterCode(IC_ASSIGN);
            ic2->u.assign.left = place;
            ic2->u.assign.right = tmp;
            CodeList c3 = new_CodeList(ic2); // 获取结构体域的地址
            return CodePlus(3,c1,c2,c3);
        }*/       // Exp DOT ID 结构体
        else if(strcmp(Exp->fir->sib->type_name, "LB") == 0) { // 数组访问
            if(strcmp(Exp->fir->fir->type_name, "ID") == 0) {// 非结构体数组
                // 获取数组地址
                Operand v1 = lookup_var(Exp->fir->fir->id);
                Operand baseAddr = new_temp();
                baseAddr->kind = Em_ADDRESS;
                InterCode ic; // ???????
                ic->u.assign.left = baseAddr;
                ic->u.assign.right = v1;
                CodeList c0 = new_CodeList(ic);
                Operand t1 = new_temp(); // t1 地址偏移量
                Operand t2 = new_temp(); // t2 结果地址
                t2->kind = Em_ADDRESS;
                CodeList c1 = translate_Exp(Exp->fir->sib->sib, t1);
                // 地址偏移量*4
                ic = new_InterCode(IR_MUL);
                ic->u.binop.result = t1;
                ic->u.binop.op1 = t1;
                ic->u.binop.op2 = new_constant(4);
                //...
            }
        }
    }
}

CodeList translate_Args(node_t* Args, ArgList argList) {
    Operand t1 = new_temp();
    CodeList c1 = translate_Exp(Args->fir, t1);
    // 返回时，根据函数参数依次访问判断，是取地址还是取值，只有数组和结构体要取地址
    // write函数的参数为NULL，要避免空指针访问
    FieldList fdom;// fdom指向函数形参列表<===========================  ??
    if(fdom != NULL && (fdom->type->kind == ARRAY || fdom->type->kind == STRUCTURE)) {
        t1->kind =  Em_ADDRESS;
    }; //当前param类型
    ArgList newArgList = (ArgList)malloc(sizeof(struct _ArgList));
    newArgList->args = t1;
    newArgList->next = argList; // newArgList->next = *argList;?????????????
    argList = newArgList; // *argList = newArgList; ????????????????????????
    if(Args->fir->sib == NULL){ 
        return c1;  // 只有一个参数直接返回
    }else{ 
        if(fdom!=NULL) fdom = fdom->tail; // write函数的参数为NULL，要避免空指针访问
        // 处理下一对函数形参及实参
        CodeList c2 = translate_Args(Args->fir->sib->sib, argList);  
        return merge(c1,c2);
    }
}

CodeList translate_Cond(node_t *Exp, Operand label_true, Operand label_false){
    if(strcmp(Exp->fir->type_name, "NOT") == 0){ // 处理NOT逻辑运算符
        return translate_Cond(Exp->fir->sib, label_false, label_true);
    }else if (Exp->fir->sib == NULL) { // 处理表达式
        Operand t1 = new_temp();
        CodeList c1 = translate_Exp(Exp,t1);
        InterCode ic = new_InterCode(IC_IFGOTO);
        //处理true分支;
        strcpy(ic->u.if_goto.relop, "!=");
        CodeList c2 = new_CodeList(ic);
        CodeList gf = create_code_Op(label_false, IC_GOTO);
        return CodePlus(3, c1, c2, gf);
    } else {
        if(strcmp(Exp->fir->sib->type_name,"RELOP") == 0) { // 处理关系运算符
        //...
        }else if(strcmp(Exp->fir->sib->type_name, "AND") == 0) { // 处理逻辑与
            Operand label1 = new_label(); // 短路运算
            CodeList c1 = translate_Cond(Exp->fir, label1, label_false);
            CodeList c2 = translate_Cond(Exp->fir->sib->sib, label_true, label_false);
            CodeList clabel1 = create_code_Op(label1, IC_LABEL);
            return CodePlus(3, c1, clabel1, c2);
        }else if(strcmp(Exp->fir->sib->type_name, "OR") == 0){ // 逻辑或
            //...
        }
    }
}

Operand new_temp() {

}

Operand new_constant(int value) {

}

Operand lookup_var(char* name) {

}

InterCode new_InterCode(enum ICKind kind) {

}

CodeList new_CodeList(InterCode ic) {

}

CodeList create_code_Op(Operand op, enum ICKind kind) {

}

CodeList concatenate(CodeList c1, CodeList c2) {

}

void insert_code(CodeList codelist) {

}

void print_IR(CodeList codelisthead, FILE* ff) {

}