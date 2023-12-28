#include "intercode.h"

int var_num,label_num,temp_num; // 新变量/新标签/新临时变量编号
CodeList code_head, code_tail;  // 双链表的首部和尾部
Variable var_head, var_tail;     // 变量表的首部和尾部

#define HASH_SIZE 0x3fff
typedef struct Hash_{
    char *name;
    Operand op;
    struct Hash_ *next;
}* Hash;

Hash opTable[HASH_SIZE + 2];

void init_opTable() {
    for (int i = 0; i < HASH_SIZE + 2; i++) {
        opTable[i] = NULL;
    }
}

void insertOp(char *name, Operand op){
    unsigned int hash = hashFunc(name);
    Hash newnode = (Hash) malloc(sizeof(struct Hash_));
    newnode->name = name; newnode->op = op; newnode->next = NULL; 
    Hash node = opTable[hash];
    if(node == NULL){
        opTable[hash] = newnode;
    } else {
        while (node->next != NULL) node = node->next;
        node->next = newnode;
    }
}

Operand Op_get(char *name) { // 从最近的field开始遍历
    unsigned int hash = hashFunc(name);
    for(Hash node = opTable[hash]; node != NULL; node = node->next){
        if(strcmp(node->name, name) == 0){
            return node->op;
        }
    }
    return NULL;
}

CodeList getIntercode(node_t* root){
    // 首先判断AST是否为空;
    // 接着判断AST结构是否正确（满足C—文法要求）;
    if (NULL == root) return NULL;
    if(strcmp(root->type_name, "Program")!= 0 ) return NULL;
    // 初始化全局变量中间代码变量等列表;
    code_head = code_tail = NULL;
    var_tail = var_head = NULL;
    // 设定变量、标签、临时变量标号;
    var_num = 0; label_num = 0; temp_num = 0;

    init_opTable();
    // 遍历AST的节点，依次调用相应的translate_X函数;
    node_t* node = root->fir;  // ExtDefList
    while(node != NULL){
        node = node->fir;
        code_head = concatenate(code_head, translate_ExtDef(node));
        //递归遍历兄弟节点;
        node = node->sib; 
    }
    return code_head;
}

void checkSpecifier(node_t* spec) {
    assert(spec && strcmp(spec->type_name, "Specifier") == 0);
    if (strcmp(spec->fir->type_name, "StructSpecifier") == 0) {
        fprintf(stderr, "Cannot translate: Code contains variables or parameters of structure type.\n");
        exit(1);
    }
}

CodeList translate_ExtDef(node_t* ExtDef){
    // 检查语法树是否为空;
    if (ExtDef == NULL) return NULL; // ??  确实有可能空 
    // 检查是否为"；";
    checkSpecifier(ExtDef->fir);
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
    if(strcmp(FunDec->fir->sib->sib->type_name, "RP") == 0){// 处理无参函数, checkp(FunDec, 3, "ID", "LP", "RP")
        // 初始化函数信息，获取函数名
        InterCode ic = new_InterCode(IC_FUNC);
        ic->u.func = FunDec->fir->id;
        CodeList c1 = new_CodeList(ic);
        return c1;
    } 
    else if(strcmp(FunDec->fir->sib->sib->type_name, "VarList") == 0){// 处理有参函数, checkp(FunDec, 4, "ID", "LP", "VarList", "RP")
        // 初始化函数信息，获取函数名
        InterCode ic = new_InterCode(IC_FUNC);  //IR_FUNC??
        ic->u.func = FunDec->fir->id;
        CodeList c1 = new_CodeList(ic);
        // 获取参数列表
        CodeList c2 = NULL;
        FieldList params = Type_get(FunDec->fir->id)->u.function->param;
        while (params != NULL) { // <==========================================TBD
            // 构造函数参数列表判断参数类型（选做内容中需要支持结构体和数组作为参数）
            if(params->type->kind == BASIC) { 
                InterCode ic0 = new_InterCode(IC_PARAM);
                ic0->u.op = lookup_var(params->name); // same as in Exp <------------------------ TOCHECK: is the name valid??????????????????
                c2 = concatenate(c2, new_CodeList(ic0));
            }
            else { // 处理数组(或者结构体)
                if (params->type->kind == STRUCTURE) { // FIXME: why go into this
                    fprintf(stderr, "Cannot translate: Code contains variables or parameters of structure type.\n");
                    exit(1);
                }
                // assert(0); //数组
                InterCode ic = new_InterCode(IC_PARAM);
                int size;
                ic->u.op = deal_array(params->name, &(size))/*array*/;
                ic->u.op->para = 1;// FIXME: it is an address -- See Exp == "ID"
                /*ic->u.arr.size = size;*/
                c2 = concatenate(c2, new_CodeList(ic));
            }
            params = params->tail;// 遍历下一个？？
        }
        return concatenate(c1, c2);
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

CodeList translate_DefList(node_t* DefList) { // <==================== check1
    CodeList ret = NULL;
    node_t* node = DefList;
    while(node != NULL){
        node = node->fir;
        CodeList def = translate_Def(node);
        ret = concatenate(ret, def); // <======== why? 
        //递归遍历兄弟节点;
        node = node->sib; 
    }
    return ret;
}

CodeList translate_Def(node_t* Def) {
    // TODO: Def -> Specifier DecList SEMI, DecList -> Dec | Dec COMMA DecList
    // Dec -> VarDec | VarDec ASSIGNOP Exp, VarDec -> ID | VarDec LB INT RB
    checkSpecifier(Def->fir);
    CodeList ret = NULL;
    node_t* DecList = Def->fir->sib;
    do {
        node_t* Dec = DecList->fir;
        /* Code: Dec */
        node_t* VarDec = Dec->fir;
        if (strcmp(VarDec->fir->type_name, "ID") == 0) {
            if (VarDec->sib != NULL) {
                ret = concatenate(ret, translate_Exp(VarDec->sib->sib, lookup_var(VarDec->fir->id))); // <------- 第一次声明和定义的时候怎么查找？？
            }
            //else return NULL; // no need to initialize them
        }
        else if (strcmp(VarDec->fir->type_name, "VarDec") == 0) {
            node_t *id = VarDec;
            while (strcmp(id->type_name, "VarDec") == 0) {
                id = id->fir;
            }
            assert(strcmp(id->type_name, "ID") == 0);
            // Operand array = lookup_var(id->id);
            // Type arr_level = array->type;
            // int size = sizeof(int);
            // while (arr_level && arr_level->kind == ARRAY) {
            //     size *= arr_level->u.array.size;
            //     arr_level = arr_level->u.array.elem;
            // }
            InterCode ic = new_InterCode(IC_DEC);
            ic->u.arr.op = deal_array(id->id, &(ic->u.arr.size))/*array*/;
            /*ic->u.arr.size = size;*/
            ret = concatenate(ret, new_CodeList(ic));
        }
        else assert(0);
        /* End here */
        if (Dec->sib != NULL) DecList = Dec->sib->sib;
        else break;
    } while (1);
    return ret;
}

CodeList translate_StmtList(node_t* StmtList) { // <==================== check2
    CodeList ret = NULL;
    node_t* node = StmtList;
    while(node != NULL){
        node = node->fir;
        CodeList stmt = translate_Stmt(node);
        ret = concatenate(ret, stmt); // <======== why? 
        //递归遍历兄弟节点;
        node = node->sib; 
    }
    return ret;
}

CodeList translate_Stmt(node_t* Stmt) {
    if (strcmp(Stmt->fir->type_name, "CompSt") == 0){ // 处理复合语句
        return translate_CompSt(Stmt->fir);
    } else if (strcmp(Stmt->fir->type_name, "Exp") == 0){ // 处理表达式
        return translate_Exp(Stmt->fir, NULL);
    } else if (strcmp(Stmt->fir->type_name, "RETURN") == 0){ // 处理返回语句
        Operand t1 = new_temp();
        CodeList c1 = translate_Exp(Stmt->fir->sib, t1);
        CodeList c2 = create_code_Op(t1, IC_RETURN);
        return concatenate(c1, c2);
    } else if (strcmp(Stmt->fir->type_name, "IF") == 0) { //  IF ELSE
        // IF LP Exp RP Stmt <ELSE Stmt>
        //CodeList stmt = NULL;
        if (Stmt->fir->sib->sib->sib->sib->sib == NULL) { // true分支
            Operand label1 = new_label();
            Operand label2 = new_label();
            CodeList code1 = translate_Cond(Stmt->fir->sib->sib, label1, label2);
            CodeList code2 = translate_Stmt(Stmt->fir->sib->sib->sib->sib);
            // stmt = concatenate(stmt, code1);
            // stmt = concatenate(stmt, create_code_Op(label1, IC_LABEL)); // <---------------------------------TBD LABEL label1
            // stmt = concatenate(stmt, code2);
            // stmt = concatenate(stmt, create_code_Op(label2, IC_LABEL)); // <---------------------------------TBD LABEL label2
            return CodePlus(4, code1, create_code_Op(label1, IC_LABEL), code2, create_code_Op(label2, IC_LABEL));
        } else { // true/false分支
            Operand label1 = new_label();
            Operand label2 = new_label();
            Operand label3 = new_label();
            CodeList code1 = translate_Cond(Stmt->fir->sib->sib, label1, label2);
            CodeList code2 = translate_Stmt(Stmt->fir->sib->sib->sib->sib);
            CodeList code3 = translate_Stmt(Stmt->fir->sib->sib->sib->sib->sib->sib);
            // stmt = concatenate(stmt, code1);
            // stmt = concatenate(stmt, create_code_Op(label1, IC_LABEL)); // <---------------------------------TBD LABEL label1
            // stmt = concatenate(stmt, code2);
            // stmt = concatenate(stmt, create_code_Op(label3, IC_GOTO)); // <---------------------------------TBD GOTO label3
            // stmt = concatenate(stmt, create_code_Op(label2, IC_LABEL)); // <---------------------------------TBD LABEL label2
            // stmt = concatenate(stmt, code3);
            // stmt = concatenate(stmt, create_code_Op(label3, IC_LABEL)); // <---------------------------------TBD LABEL label3
            return CodePlus(7, code1, create_code_Op(label1, IC_LABEL), code2, create_code_Op(label3, IC_GOTO), create_code_Op(label2, IC_LABEL), code3, create_code_Op(label3, IC_LABEL));
        }
        // return stmt;
    } else if (strcmp(Stmt->fir->type_name, "WHILE") == 0) { // while语句处理
        //CodeList stmt = NULL;
        Operand label1 = new_label();
        Operand label2 = new_label();
        Operand label3 = new_label();
        CodeList code1 = translate_Cond(Stmt->fir->sib->sib, label2, label3);
        CodeList code2 = translate_Stmt(Stmt->fir->sib->sib->sib->sib);
        // stmt = concatenate(stmt, create_code_Op(label1, IC_LABEL)); // <---------------------------------TBD LABEL label1
        // stmt = concatenate(stmt, code1);
        // stmt = concatenate(stmt, create_code_Op(label2, IC_LABEL)); // <---------------------------------TBD LABEL label2
        // stmt = concatenate(stmt, code2);
        // stmt = concatenate(stmt, create_code_Op(label1, IC_GOTO)); // <---------------------------------TBD GOTO label1
        // stmt = concatenate(stmt, create_code_Op(label3, IC_LABEL)); // <---------------------------------TBD  LABEL label3
        return CodePlus(6, create_code_Op(label1, IC_LABEL), code1, create_code_Op(label2, IC_LABEL), code2, create_code_Op(label1, IC_GOTO), create_code_Op(label3, IC_LABEL));
    }
    return NULL;
}

CodeList translate_Exp(node_t* Exp, Operand place) { // place表示值
    assert(Exp && strcmp(Exp->type_name, "Exp") == 0);
    if(strcmp(Exp->fir->type_name, "INT") == 0) { //INT
        int val = Exp->fir->num.type_int;
        InterCode ic = new_InterCode(IC_ASSIGN);
        ic->u.assign.left = place; // 构建左值
        ic->u.assign.right = new_constant(val); // 构建常量
        return new_CodeList(ic);
    }
    else if(strcmp(Exp->fir->type_name, "FLOAT") == 0) {
        assert(0);
        return NULL;
    }
    else if(strcmp(Exp->fir->type_name, "ID") == 0){
        if(Exp->fir->sib == NULL){ // 处理标识符
            Operand op = lookup_var(Exp->fir->id); // 查找标识符
            if(op->kind == Em_ARR/* || op->kind == Em_STRUCT*/){ // 数组or结构体返回地址 TODO: 什么时候出现这种情况？答：ArgList调用translate_Exp
                InterCode ic = NULL;  //  在下面的LB中会用到+ASSIGNOP的一个情况
                if (op->para == 1) ic = new_InterCode(IC_ASSIGN);  // 
                else ic = new_InterCode(IC_GET_ADDR);
                ic->u.assign.left = place;
                ic->u.assign.right = op;
                return new_CodeList(ic);
            }
            else { // 一般变量直接赋值(地址也是) // <======================================= 地址怎么出现的
                InterCode ic = new_InterCode(IC_ASSIGN);
                ic->u.assign.left = place;
                ic->u.assign.right = op;
                return new_CodeList(ic);
            }
        }
        else if (strcmp(Exp->fir->sib->type_name, "LP") == 0) { // 函数调用
            if (strcmp(Exp->fir->sib->sib->type_name, "Args") == 0){ 
                //Function fun = Type_get(Exp->fir->id)->u.function;  //直接找id不是更好吗？？
                // 获取函数参数列表并处理实参
                ArgList argList = NULL; // <======================================= ?? 
                CodeList c1 = translate_Args(Exp->fir->sib->sib, &argList);
                if(strcmp(Exp->fir->id/*fun->name*/, "write") == 0) { // TODO: 事先插入write
                    InterCode ic = new_InterCode(IC_WRITE);
                    ic->u.op = argList->args;
                    // + [place := #0]  <------------------------------------------??
                    return concatenate(c1, new_CodeList(ic));
                } else { // 用户自定义函数
                    CodeList c2 = NULL;
                    while (argList != NULL) {
                        c2 = concatenate(c2, create_code_Op(argList->args, IC_ARG));  //创建实参压栈IR;
                        argList = argList->next;
                    }
                    //创建函数调用IR;
                    InterCode ic = new_InterCode(IC_CALL);
                    ic->u.call.func = Exp->fir->id/*fun->name*/;
                    ic->u.call.result = place;
                    return CodePlus(3, c1, c2, new_CodeList(ic));
                    //合并操作;
                }
            }
            else {
                //Function fun = Type_get(Exp->fir->id)->u.function;  //直接找id不是更好吗？？--但还是要插入，否则语义错误
                if(strcmp(Exp->fir->id/*fun->name*/, "read") == 0) { // TODO: 事先插入read
                    InterCode ic = new_InterCode(IC_READ);
                    ic->u.op = place; // <====================== READ place??
                    return new_CodeList(ic);
                } else { // 用户自定义函数
                    InterCode ic = new_InterCode(IC_CALL);
                    ic->u.call.func = Exp->fir->id/*fun->name*/;
                    ic->u.call.result = place;
                    return new_CodeList(ic);
                }
            }
        }
        else assert(0);
    }
    else if (strcmp(Exp->fir->type_name, "LP") == 0) {
        return translate_Exp(Exp->fir->sib, place);
    }
    else if (strcmp(Exp->fir->type_name, "MINUS") == 0) {
        Operand t1 = new_temp();
        CodeList c1 = translate_Exp(Exp->fir->sib, t1);
        InterCode ic = new_InterCode(IC_MINUS);
        ic->u.binop.op1 = new_constant(0); ic->u.binop.op2 = t1;
        ic->u.binop.result = place;
        return concatenate(c1, new_CodeList(ic));
    }
    else if (strcmp(Exp->fir->type_name, "NOT") == 0 || strcmp(Exp->fir->sib->type_name, "RELOP") == 0 || strcmp(Exp->fir->sib->type_name, "AND") == 0 || strcmp(Exp->fir->sib->type_name, "OR") == 0) {
        Operand label1 = new_label();
        Operand label2 = new_label();
        InterCode ic0 = new_InterCode(IC_ASSIGN);
        ic0->u.assign.left = place; ic0->u.assign.right = new_constant(0);
        CodeList c0 = new_CodeList(ic0);
        CodeList c1 = translate_Cond(Exp, label1, label2);
        CodeList c2 = create_code_Op(label1, IC_LABEL);
        InterCode ic1 = new_InterCode(IC_ASSIGN);
        ic0->u.assign.left = place; ic0->u.assign.right = new_constant(1);
        c2 = concatenate(c2, new_CodeList(ic1));
        return CodePlus(4, c0, c1, c2, create_code_Op(label2, IC_LABEL));
    }
    else if (strcmp(Exp->fir->sib->type_name, "ASSIGNOP") == 0) {
        //(place = ) ID = Exp => t1 = Exp, ID = t1, place = ID// <============= TODO: what if place is Em_ARR, like a[4] = 2  
        //assert(0);// FIXME: if a[2] is not a int
        if (strcmp(Exp->fir->fir->type_name, "ID") == 0) {
            Operand rval = new_temp();
            CodeList c1 = translate_Exp(Exp->fir->sib->sib, rval);
            // variable or array (with no LB & RB)
            Operand lval = lookup_var(Exp->fir->fir->id);
            InterCode ic0 = NULL, ic1 = NULL;
            if (lval->kind == Em_ARR) { // TODO:需要？？
                // rval = Exp, ID = rval, place = &IDInterCode ic0 = new_InterCode(IC_ASSIGN), ic1 = new_InterCode(IC_ASSIGN);
                ic0 = new_InterCode(IC_ASSIGN), ic1 = new_InterCode(IC_ASSIGN); // Situation: int array[5], a[5];a = array;
                lval->para = 1; // FIXME: 假象的PARA
            }
            else {
                ic0 = new_InterCode(IC_ASSIGN), ic1 = new_InterCode(IC_ASSIGN);
            }
            ic0->u.assign.left = lval;
            ic0->u.assign.right = rval;
            ic1->u.assign.left = place;
            ic1->u.assign.right = lval;
            CodeList c2 = concatenate(new_CodeList(ic0), new_CodeList(ic1)); // [variable.name := rval] + [place := variable.name]
            return concatenate(c1, c2);
        }
        else {
            // assert(0);// TODO:
            Operand rval = new_temp();
            CodeList c1 = translate_Exp(Exp->fir->sib->sib, rval);
            Operand lval = new_temp();
            lval->isaddr = 1;
            CodeList c2 = translate_Exp(Exp->fir, lval);
            InterCode ic0 = NULL, ic1 = NULL;
            // a[1] = 1 -> *t = 1
            // a[1] = b[2] (address) -> t1 = t2
            if (rval->isaddr) {
                ic0 = new_InterCode(IC_ASSIGN);
                ic1 = new_InterCode(IC_ASSIGN);
            }
            else {
                ic0 = new_InterCode(IC_STORE);
                ic1 = new_InterCode(IC_DEREF);
            }
            ic0->u.assign.left = lval;
            ic0->u.assign.right = rval;
            ic1->u.assign.left = place;
            ic1->u.assign.right = lval;
            //CodeList c2 = concatenate(new_CodeList(ic0), new_CodeList(ic1)); 
            return CodePlus(4, c1, c2, new_CodeList(ic0), new_CodeList(ic1));
        }
    }
    else if (strcmp(Exp->fir->sib->type_name, "PLUS") == 0 || strcmp(Exp->fir->sib->type_name, "MINUS") == 0 || strcmp(Exp->fir->sib->type_name, "STAR") == 0 || strcmp(Exp->fir->sib->type_name, "DIV") == 0) {
        Operand t1 = new_temp();
        Operand t2 = new_temp();
        CodeList c1 = translate_Exp(Exp->fir, t1);
        CodeList c2 = translate_Exp(Exp->fir->sib->sib, t2);
        InterCode ic;
        if (strcmp(Exp->fir->sib->type_name, "PLUS") == 0) ic = new_InterCode(IC_PLUS);
        else if (strcmp(Exp->fir->sib->type_name, "MINUS") == 0) ic = new_InterCode(IC_MINUS);
        else if (strcmp(Exp->fir->sib->type_name, "STAR") == 0) ic = new_InterCode(IC_MUL);
        else if (strcmp(Exp->fir->sib->type_name, "DIV") == 0) ic = new_InterCode(IC_DIV);
        else assert(0);
        ic->u.binop.op1 = t1, ic->u.binop.op2 = t2;
        ic->u.binop.result = place;
        return CodePlus(3, c1, c2, new_CodeList(ic));
    }
    else if (strcmp(Exp->fir->sib->type_name, "LB") == 0) {
        //if(strcmp(Exp->fir->fir->type_name, "ID") == 0) {// 非结构体数组
            // // 获取数组地址 baseAddr = v1
            // Operand v1 = lookup_var(Exp->fir->fir->id);
            // Operand baseAddr = new_temp();
            // //  baseAddr->kind = Em_ADDRESS;  //<=====================FIXME:
            // InterCode ic1 = new_InterCode(IC_GET_ADDR);
            // ic1->u.assign.left = baseAddr;
            // ic1->u.assign.right = v1;
            // CodeList c0 = new_CodeList(ic1);
            // ==>> now translate into
            Operand baseAddr = new_temp();
            CodeList c0 = translate_Exp(Exp->fir, baseAddr);

            // t1 = Exp2
            Operand t1 = new_temp(); // t1 地址偏移量
            Operand t2 = new_temp(); // t2 结果地址
            //  t2->kind = Em_ADDRESS;  //<=====================FIXME:
            CodeList c1 = translate_Exp(Exp->fir->sib->sib, t1);
            // 地址偏移量*4 t1 = t1 * 4
            InterCode ic2 = new_InterCode(IC_MUL);
            ic2->u.binop.result = t1;
            ic2->u.binop.op1 = t1;
            /* calculate the size */
            int size;
            int level = 0;
            node_t* exp = Exp;
            while (strcmp(exp->fir->type_name, "ID") != 0) {
                exp = exp->fir;
                level++;
            }
            Type arrayType = deal_array(exp->fir->id, &size)->type;
            //printf("array size = %d\n", size);
            while (level--) {
                size /= arrayType->u.array.size;
                arrayType = arrayType->u.array.elem;
            }
            //printf("size = %d\n", size);
            /**/
            ic2->u.binop.op2 = new_constant(size);
            // 地址+偏移 t2 = baseAddr + t1
            InterCode ic3 = new_InterCode(IC_PLUS);
            ic3->u.binop.result = t2;
            ic3->u.binop.op1 = baseAddr; ic3->u.binop.op2 = t1;
            // 赋值 place = *t2 or place = t2 (不是末尾)
            InterCode ic4;
            if (arrayType->kind == BASIC && arrayType->u.basic == 1/*tail*/) {
                if (place && place->isaddr == 1) ic4 = new_InterCode(IC_ASSIGN); // for ASSIGNOP, a[1] = **;
                else ic4 = new_InterCode(IC_DEREF);
            }
            else {
                if (place) place->isaddr = 1;
                ic4 = new_InterCode(IC_ASSIGN);
            }
            ic4->u.assign.left = place;
            ic4->u.assign.right = t2;
            return CodePlus(5, c0, c1, new_CodeList(ic2), new_CodeList(ic3), new_CodeList(ic4));
        // }
        // else { // TODO:
        //     assert(0);
        //     return NULL;
        // }
    }
    else if (strcmp(Exp->fir->sib->type_name, "DOT") == 0) {
        assert(0); // <-----------------------------------------------------------maybe should remove it
        return NULL;
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
    }
    else assert(0);
}

CodeList translate_Args(node_t* Args, ArgList* argList) {
    Operand t1 = new_temp();
    CodeList c1 = translate_Exp(Args->fir, t1);
    ArgList newArgList = (ArgList)malloc(sizeof(struct _ArgList));
    newArgList->args = t1;
    newArgList->next = *argList;
    *argList = newArgList;
    if (Args->fir->sib == NULL) {
        return c1;
    }
    else {
        CodeList c2 = translate_Args(Args->fir->sib->sib, argList);
        return concatenate(c1, c2);
    }
    // Operand t1 = new_temp();
    // CodeList c1 = translate_Exp(Args->fir, t1);
    // // 返回时，根据函数参数依次访问判断，是取地址还是取值，只有数组和结构体要取地址
    // // write函数的参数为NULL，要避免空指针访问
    // FieldList fdom = Args;// fdom指向函数形参列表<===========================  ??
    // if(fdom != NULL && (fdom->type->kind == ARRAY/* || fdom->type->kind == STRUCTURE*/)) { // <--------- ?? 
    //     t1->kind =  Em_ADDRESS;
    // }; //当前param类型
    // ArgList newArgList = (ArgList)malloc(sizeof(struct _ArgList));
    // newArgList->args = t1;
    // newArgList->next = *argList;  //?????????????
    // *argList = newArgList;  //????????????????????????
    // if(Args->fir->sib == NULL){ 
    //     return c1;  // 只有一个参数直接返回
    // }
    // else { 
    //     if(fdom != NULL) fdom = fdom->tail; // write函数的参数为NULL，要避免空指针访问 ???????
    //     // 处理下一对函数形参及实参
    //     CodeList c2 = translate_Args(Args->fir->sib->sib, argList);  
    //     return concatenate(c1, c2);
    // }
}

CodeList translate_Cond(node_t *Exp, Operand label_true, Operand label_false) {
    if (strcmp(Exp->fir->type_name, "NOT") == 0) { // 处理NOT逻辑运算符
        return translate_Cond(Exp->fir->sib, label_false, label_true);
    } else if (Exp->fir->sib == NULL) { // 处理表达式
        Operand t1 = new_temp();
        CodeList c1 = translate_Exp(Exp, t1);

        // CodeList c2 = [IF t1 != #0 GOTO label_true]
        InterCode ic = new_InterCode(IC_IFGOTO);
        strcpy(ic->u.if_goto.relop, "!=");
        ic->u.if_goto.t1 = t1; ic->u.if_goto.t2 = new_constant(0);
        ic->u.if_goto.label = label_true;
        CodeList c2 = new_CodeList(ic);

        CodeList gf = create_code_Op(label_false, IC_GOTO);
        return CodePlus(3, c1, c2, gf);
    } else {
        if (strcmp(Exp->fir->sib->type_name,"RELOP") == 0) { // 处理关系运算符
            Operand t1 = new_temp();
            Operand t2 = new_temp();
            CodeList c1 = translate_Exp(Exp->fir, t1);
            CodeList c2 = translate_Exp(Exp->fir->sib->sib, t2);

            //op = get_relop(RELOP);
            //CodeList c3 = [IF t1 op t2 GOTO label_true];
            InterCode ic = new_InterCode(IC_IFGOTO);
            strcpy(ic->u.if_goto.relop, Exp->fir->sib->id);
            ic->u.if_goto.t1 = t1; ic->u.if_goto.t2 = t2; 
            ic->u.if_goto.label = label_true;
            CodeList c3 = new_CodeList(ic);

            CodeList gf = create_code_Op(label_false, IC_GOTO);
            return CodePlus(4, c1, c2, c3, gf);
        } else if (strcmp(Exp->fir->sib->type_name, "AND") == 0) { // 处理逻辑与
            Operand label1 = new_label(); // 短路运算
            CodeList c1 = translate_Cond(Exp->fir, label1, label_false);
            CodeList c2 = translate_Cond(Exp->fir->sib->sib, label_true, label_false);
            CodeList clabel1 = create_code_Op(label1, IC_LABEL);
            return CodePlus(3, c1, clabel1, c2);
        } else if (strcmp(Exp->fir->sib->type_name, "OR") == 0) { // 逻辑或
            Operand label1 = new_label(); // 短路运算
            CodeList c1 = translate_Cond(Exp->fir, label_true, label1);
            CodeList c2 = translate_Cond(Exp->fir->sib->sib, label_true, label_false);
            CodeList clabel1 = create_code_Op(label1, IC_LABEL);
            return CodePlus(3, c1, clabel1, c2);
        }
        else {
            //assert(0); 其他表达式
            Operand t1 = new_temp();
            CodeList c1 = translate_Exp(Exp, t1);
            //printf("here\n");
            // CodeList c2 = [IF t1 != #0 GOTO label_true]
            InterCode ic = new_InterCode(IC_IFGOTO);
            strcpy(ic->u.if_goto.relop, "!=");
            ic->u.if_goto.t1 = t1; ic->u.if_goto.t2 = new_constant(0);
            ic->u.if_goto.label = label_true;
            CodeList c2 = new_CodeList(ic);

            CodeList gf = create_code_Op(label_false, IC_GOTO);
            return CodePlus(3, c1, c2, gf);
        }
    }
}

Operand newOperand(enum OpKind kind, int value) {
    Operand op = (Operand)malloc(sizeof(struct _Operand));
    op->kind = kind;
    op->isaddr = 0;//default
    op->para = 0;
    switch (kind) {
        case Em_VARIABLE: op->u.varno = value; break;
        case Em_CONSTANT: op->u.val = value; break;
        case Em_ADDRESS: break;  // need????????????????????????
        case Em_LABEL: op->u.labelno = value; break;
        case Em_ARR: op->u.varno = value; break;  // need????????????????????????
        //case Em_STRUCT: break;
        case Em_TEMP: op->u.tempno = value; break;
        default: assert(0); 
    }
}

Operand new_var() {
    var_num++;
    return newOperand(Em_VARIABLE, var_num);
}

Operand new_arr() {
    var_num++;
    return newOperand(Em_ARR, var_num);
}

Operand new_temp() {
    temp_num++;
    return newOperand(Em_TEMP, temp_num);
}

Operand new_constant(int value) {
    return newOperand(Em_CONSTANT, value);
}

Operand new_label() {
    label_num++;
    return newOperand(Em_LABEL, label_num);
}

Operand lookup_var(char* name) {
    Type type = Type_get(name);
    assert(type);
    if (type->kind == BASIC) {
        assert(type->u.basic == 1);
        Operand op = Op_get(name);
        if (op != NULL) return op;
        op = new_var();
        insertOp(name, op);
        return op;
    }
    else if (type->kind == ARRAY) {
        Operand op = Op_get(name);
        if (op != NULL) return op;
        op = new_arr();
        insertOp(name, op);
        op->type = type;
        return op;
    }
    else assert(0);
}

Operand deal_array(char* name, int* size) {
    Operand array = lookup_var(name);
    Type arr_level = array->type;
    *size = sizeof(int);
    while (arr_level && arr_level->kind == ARRAY) {
        *size *= arr_level->u.array.size;
        arr_level = arr_level->u.array.elem;
    }
    return array;
}

InterCode new_InterCode(enum ICKind kind) {
    InterCode ic = (InterCode)malloc(sizeof(struct _InterCode));
    ic->kind = kind;
    // TODO <=======================
    return ic;
}

CodeList new_CodeList(InterCode ic) {
    CodeList cl = (CodeList)malloc(sizeof(struct _CodeList));
    cl->code = ic;
    cl->prev = cl; // <==========================
    cl->next = cl; // <==========================
    return cl;
}

CodeList create_code_Op(Operand op, enum ICKind kind) {
    InterCode ic = new_InterCode(kind);
    ic->u.op = op; // 单操作数应该就是这样吧
    return new_CodeList(ic);
}

CodeList concatenate(CodeList c1, CodeList c2) {
    //  注意ret = concatenate(ret, code)的使用，应当返回第一个CodeList的指针. 
    //  同时注意，ret==NULL的情况<=======可以在调用的地方保证ret为第一个非NULL吗————万一其后的任何一个出现NULL怎么办？————都要判断
        // 万一两个NULL怎么办
    if (c1 == NULL) return c2; // c2 == NULL??
    if (c2 == NULL) return c1;
    CodeList tail = c2->prev;
    tail->next = c1;
    c2->prev = c1->prev;
    c1->prev->next = c2;
    c1->prev = tail;
    return c1;
}

CodeList CodePlus(int n, ...) { // ??????????????????????????????????
    // TODO: check the number n
    CodeList ret = NULL;
    va_list ap;
    va_start(ap, n);
    for(int i = 0; i < n; i++) {
        ret = concatenate(ret, va_arg(ap, CodeList));
    }
    va_end(ap);
    return ret;
}

void oprandToString(Operand op, char* buffer) {
    switch (op->kind) {
        case Em_VARIABLE: sprintf(buffer, "v%d", op->u.varno); break;
        case Em_CONSTANT: sprintf(buffer, "#%d", op->u.val); break;
        case Em_ADDRESS: assert(0);/*FIXME:*/ break; // 地址（&var）
        case Em_LABEL: sprintf(buffer, "label%d", op->u.labelno); break;
        case Em_ARR: sprintf(buffer, "v%d", op->u.varno); break; // 数组（arr[2]）
        case Em_TEMP: sprintf(buffer, "t%d", op->u.tempno); break;
        default: assert(0);
    }
    // return buffer;
}

void get_IR(CodeList cur, FILE* ff) {
    char op_name[32], op_name1[32], op_name2[32];
    switch (cur->code->kind) {
        case IC_ASSIGN: if (cur->code->u.assign.left == NULL) return; 
                        oprandToString(cur->code->u.assign.left, op_name);
                        oprandToString(cur->code->u.assign.right, op_name1);
                        fprintf(ff, "%s := %s\n", op_name, op_name1); break;
        case IC_LABEL: oprandToString(cur->code->u.op, op_name);
                        fprintf(ff, "LABEL %s :\n", op_name); break;
        case IC_PLUS: oprandToString(cur->code->u.binop.result, op_name); // TODO: need if (cur->code->u.assign.left == NULL) return; ??
                        oprandToString(cur->code->u.binop.op1, op_name1);
                        oprandToString(cur->code->u.binop.op2, op_name2);
                        fprintf(ff, "%s := %s + %s\n", op_name, op_name1, op_name2); break;
        case IC_MINUS: oprandToString(cur->code->u.binop.result, op_name);
                        oprandToString(cur->code->u.binop.op1, op_name1);
                        oprandToString(cur->code->u.binop.op2, op_name2);
                        fprintf(ff, "%s := %s - %s\n", op_name, op_name1, op_name2); break;
        case IC_MUL: oprandToString(cur->code->u.binop.result, op_name);
                        oprandToString(cur->code->u.binop.op1, op_name1);
                        oprandToString(cur->code->u.binop.op2, op_name2);
                        fprintf(ff, "%s := %s * %s\n", op_name, op_name1, op_name2); break;
        case IC_DIV: oprandToString(cur->code->u.binop.result, op_name);
                        oprandToString(cur->code->u.binop.op1, op_name1);
                        oprandToString(cur->code->u.binop.op2, op_name2);
                        fprintf(ff, "%s := %s / %s\n", op_name, op_name1, op_name2); break;
        case IC_FUNC: fprintf(ff, "FUNCTION %s :\n", cur->code->u.func); break;
        //
        case IC_GET_ADDR:  if (cur->code->u.assign.left == NULL) return; 
                            oprandToString(cur->code->u.assign.left, op_name);
                            oprandToString(cur->code->u.assign.right, op_name1);
                            fprintf(ff, "%s := &%s\n", op_name, op_name1); /*TODO: or in operand??*/break;
        case IC_DEREF:  if (cur->code->u.assign.left == NULL) return; 
                            oprandToString(cur->code->u.assign.left, op_name);
                            oprandToString(cur->code->u.assign.right, op_name1);
                            fprintf(ff, "%s := *%s\n", op_name, op_name1); /*TODO: or in operand??*/break;
        case IC_STORE:  if (cur->code->u.assign.left == NULL) return; 
                            oprandToString(cur->code->u.assign.left, op_name);
                            oprandToString(cur->code->u.assign.right, op_name1);
                            fprintf(ff, "*%s := %s\n", op_name, op_name1); /*TODO: or in operand??*/break;
        //
        case IC_IFGOTO: oprandToString(cur->code->u.if_goto.t1, op_name);
                        oprandToString(cur->code->u.if_goto.t2, op_name1);
                        oprandToString(cur->code->u.if_goto.label, op_name2);
                        fprintf(ff, "IF %s %s %s GOTO %s\n", op_name, cur->code->u.if_goto.relop, op_name1, op_name2); /*TODO: or in operand??*/break;
        case IC_GOTO: oprandToString(cur->code->u.op, op_name); fprintf(ff, "GOTO %s\n", op_name); break;
        //
        case IC_CALL: if (cur->code->u.call.result == NULL) cur->code->u.call.result = new_temp(); /* 自己创作一个返回值 */ oprandToString(cur->code->u.call.result, op_name);
                        fprintf(ff, "%s := CALL %s\n", op_name, cur->code->u.call.func); break;
        case IC_PARAM: oprandToString(cur->code->u.op, op_name); fprintf(ff, "PARAM %s\n", op_name); break;
        case IC_ARG: oprandToString(cur->code->u.op, op_name); fprintf(ff, "ARG %s\n", op_name); break;
        case IC_READ: oprandToString(cur->code->u.op, op_name); fprintf(ff, "READ %s\n", op_name); break;
        case IC_WRITE: oprandToString(cur->code->u.op, op_name); fprintf(ff, "WRITE %s\n", op_name); break;
        case IC_RETURN: oprandToString(cur->code->u.op, op_name); fprintf(ff, "RETURN %s\n", op_name); break;
        case IC_DEC: oprandToString(cur->code->u.arr.op, op_name); fprintf(ff, "DEC %s %d\n", op_name, cur->code->u.arr.size); break;
        default: assert(0);
    }
}

void print_IR(CodeList codelisthead, FILE* ff) {
    assert(ff);
    if (NULL == codelisthead) {
        return;
    }
    CodeList cur = codelisthead;
    do {
        get_IR(cur, ff);
        cur = cur->next;
    } while (cur != codelisthead);
}