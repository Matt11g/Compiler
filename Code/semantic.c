#include "semantic.h"

void semantic_check(node_t* root) {
    /* init */
    initTable();
    ExtDefList(root->fir);
    /* memory management */
}

void ExtDefList(node_t* node) {
    // ExtDefList -> ExtDef ExtDefList | epsilon
    if (NULL == node) return;
    ExtDef(node->fir);
    if (NULL == node->fir->sib) return;
    ExtDefList(node->fir->sib);
}

void ExtDef(node_t* node) {
    assert(NULL != node);  //if (NULL == node) return;
    // ExtDef -> Specifier ExtDecList SEMI
    // ExtDef -> Specifier SEMI
    // ExtDef -> Specifier FunDec CompSt
    node_t* spec = node->fir, *n = spec->sib;
    if (strcmp(n->type_name, "SEMI") == 0) {
        ExtSpecifier(spec);
    }
    else if (strcmp(n->type_name, "FunDec") == 0) {
        Type varType = Specifier(spec);
        Function func = FunDec(n, varType);
        CompSt(n->sib, varType);
    }
    else if (strcmp(n->type_name, "ExtDecList") == 0) {
        Type varType = Specifier(spec);
        if (NULL != varType) {/*暂时就这样吧*/
            ExtDecList(n, varType, 0); //WHAT??????????
        }
    }
    

    // ExtDecList -> VarDec | VarDec COMMA ExtDecList
    
    // FunDec -> ID LP VarList RP | ID LP RP

    /* Specifier Type t */
    /* Other info sibc */

    // 判断节点是否为空;
    // 获取node的specifier类型t;
    // 获取node的其他节点信息sibc;
    // // 如果sibc是全局变量ExtDecList
    // ExtDecList(sibc, t);
    // // 如果sibc是结构体定义,t就是结构体类型
    // // 如果sibc是函数定义
    // 提取函数参数，返回值类型t，函数名;
    // 声明函数;
}

FieldList ExtDecList(node_t* node, Type type, int structflag) {
    // ExtDecList -> VarDec | VarDec COMMA ExtDecList
    assert(node && strcmp(node->type_name, "ExtDecList") == 0);
    node_t* n = node->fir;
    FieldList f = VarDec(n, type, structflag);
    if (NULL != n->sib) f->tail = ExtDecList(n->sib->sib, type, structflag);
    else f->tail = NULL;
    return f;
}

void ExtSpecifier(node_t* node) {
    // for ExtDef -> Specifier SEMI
    node_t* n = node->fir;
    if (strcmp(n->type_name, "TYPE") == 0) {
        return; // int;的情况
    }
    else if (strcmp(n->type_name, "StructSpecifier") == 0) {
        ExtStructSpecifier(n);
    }
    else assert(0);
}

Type Specifier(node_t* node) {
    // Specifier -> TYPE | StructSpecifier
    // StructSpecifier -> STRUCT OptTag LC DefList RC | STRUCT Tag
    // TYPE -> int | float
    node_t* n = node->fir; assert(n);
    if (strcmp(n->type_name, "TYPE") == 0) {
        if (strcmp(n->id, "int") == 0) return newType(INTType);
        else if (strcmp(n->id, "float") == 0) return newType(FLOATType);
        else assert(0);
    }
    else if (strcmp(n->type_name, "StructSpecifier") == 0) {
        return StructSpecifier(n);
    }
    else assert(0);
}

void ExtStructSpecifier(node_t* node) {
    // StructSpecifier -> STRUCT OptTag LC DefList RC | STRUCT TAG
    // OptTag -> ID | epsilon, Tag -> ID
    node_t *n = node->fir; assert(n);
    if (strcmp(n->sib->type_name, "Tag") == 0) { // STRUCT TAG, 声明  
        return; //////////////////////////////////////////////////////////
    }
    else if (strcmp(n->sib->type_name, "OptTag") == 0) { //定义
        if (checkField(n->sib->fir->id)) { // DEBUG： 对类型的定义可否在语句块内重新定义？
            printf("Error type 16 at Line %d: multiple definitions of struct type\n", n->sib->fir->lineno); // 结构体重名
            return; // DEBUG: 还是不返回？？
        }
        Type sType = newType(StructType); //WRONG, TODO-------------------------------------------<<
        assert(strcmp(n->sib->fir->type_name, "ID") == 0);
        strcpy(sType->u.structure.name, n->sib->fir->id);

        insertSymbol(sType->u.structure.name, sType); // Zn: 先insert结构体的名字便于调用
        newScope();
        sType->u.structure.domain = DefList(n->sib->sib->sib, 1);  
        deleteScope(); //会不会把一些重要的Type free掉了：struct有FieldList，但还是有风险；函数直接free没问题（可见两者有区别）
    }
    else if (strcmp(n->sib->type_name, "LC") == 0) { // 匿名结构体!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        /*newScope*/
        DefList(n->sib->sib, 1);  
        /*deleteScope*/ //会不会把一些重要的Type free掉了：struct有FieldList，但还是有风险；函数直接free没问题（可见两者有区别）

        /*condition??*/ //insertSymbol(sType->u.structure.name, sType);//<-----------------------
    }
    else assert(0);

}

Type StructSpecifier(node_t* node) {
    // StructSpecifier -> STRUCT OptTag LC DefList RC | STRUCT TAG
    // OptTag -> ID | epsilon, Tag -> ID
    node_t *n = node->fir; assert(n);
    if (strcmp(n->sib->type_name, "Tag") == 0) { // STRUCT TAG, 声明  
        Type type = Type_get(n->sib->fir->id);
        if (type == NULL || type->kind != STRUCTURE) { //checkSymbol(n->sib->fir->id) == 0好还是Type_get好呢
            printf("Error type 17 at Line %d: undefined struct type\n", n->sib->fir->lineno); // 结构体重名
            return NULL;
        }
        // DEBUG:  ..  一些问题
        return type;
    }
    else if (strcmp(n->sib->type_name, "OptTag") == 0) { //定义
        if (checkField(n->sib->fir->id)) { // DEBUG： 对类型的定义可否在语句块内重新定义？
            printf("Error type 16 at Line %d: multiple definitions of struct type\n", n->sib->fir->lineno); // 结构体重名
            return NULL; // DEBUG: 还是不返回？？
        }
        Type sType = newType(StructType); //WRONG, TODO-------------------------------------------<<
        assert(strcmp(n->sib->fir->type_name, "ID") == 0);
        strcpy(sType->u.structure.name, n->sib->fir->id);

        insertSymbol(sType->u.structure.name, sType); // Zn: 先insert结构体的名字便于调用
        newScope();
        sType->u.structure.domain = DefList(n->sib->sib->sib, 1);  
        deleteScope(); //会不会把一些重要的Type free掉了：struct有FieldList，但还是有风险；函数直接free没问题（可见两者有区别）

        return sType;
    }
    else if (strcmp(n->sib->type_name, "LC") == 0) { // 匿名结构体!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        Type sType = newType(StructType); //WRONG, TODO-------------------------------------------<<
        srand((unsigned)time(NULL));
        for (int i = 0; i < 32; i++) {
            sType->u.structure.name[i] = rand() % 26 + 'a';
        }
        sType->u.structure.name[32] = '\0';
        newScope();
        insertSymbol(sType->u.structure.name, sType);
        sType->u.structure.domain = DefList(n->sib->sib, 1);  
        deleteScope(); //会不会把一些重要的Type free掉了：struct有FieldList，但还是有风险；函数直接free没问题（可见两者有区别）

        /*condition??*/
        return sType;
    }
    else assert(0);
}

FieldList Def(node_t* node, int structflag) {
    // Def -> Specifier DecList SEMI
    node_t* n = node->fir;
    Type spec = Specifier(n);
    return DecList(n->sib, spec, structflag);
}

FieldList DefList(node_t* node, int structflag) { // 可能在STRUCT里 / CompSt
    // DefList -> Def DefList | epsilon
    if (NULL == node) return NULL; // ???????????????????????????????
    assert(strcmp(node->type_name, "DefList") == 0);
    node_t* n = node->fir;
    FieldList f = Def(n, structflag);
    assert(f);
    if (NULL != n->sib) f->tail = DefList(n->sib, structflag);
    else f->tail = NULL;
    return f;
    // DEBUG: 检查Type_get_f, 是否需要完善，调用是否正确
}

void CompSt(node_t* node, Type ntype) {
    // CompSt -> LC DefList StmtList RC
    /*newScope()*/ // 如果是function的CompSt则好像不需要，struct的CompSt需要
    assert(node && strcmp(node->type_name, "CompSt") == 0); //<---------------------------均可加入
    node_t* n = node->fir;
    if (NULL == node) return;
    if (strcmp(n->sib->type_name, "RC") == 0) {
        deleteScope();//<--------------------------------------
        return;
    }
    else if (strcmp(n->sib->type_name, "DefList") == 0) {
        DefList(n->sib, 0/*NULL??, 不在struct里*/);
        if (strcmp(n->sib->sib->type_name, "StmtList") == 0) StmtList(n->sib->sib, ntype);
        deleteScope();//<--------------------------------------
    }
    else if (strcmp(n->sib->type_name, "StmtList") == 0) {
        StmtList(n->sib, ntype);
        deleteScope();//<--------------------------------------
    }
    else assert (0);
    /*deleteScope()*/
}

void StmtList(node_t* node, Type ntype) {
    if (NULL == node) return;
    Stmt(node->fir, ntype);
    if (NULL == node->fir) return;
    StmtList(node->fir->sib, ntype);
}

void Stmt(node_t* node, Type ntype) {
    if (NULL == node) return;
    node_t *n = node->fir;
    if (strcmp(n->type_name, "Exp") == 0) {
        Exp(n);
        //freeType(free_type);
    } else if (strcmp(n->type_name, "CompSt") == 0) {
        CompSt(n, ntype);
    }
    else if (strcmp(n->type_name, "RETURN") == 0) {
        Type expType = Exp(n->sib);
        // Exp fails don't report return type error
        if (/*(NULL == expType) ||*/ (expType && Type_check(expType, ntype) == 0)) {
            printf("Error type 8 at Line %d: Type mismatched for return. \n", n->lineno); // 返回值错误
        }
        //freeType(expType); 可能存在内存泄漏，例如 return 1 || 但如果不copyType的话free会导致函数的返回值被删除 a = m()  return m()
    }
    else if (strcmp(n->type_name, "WHILE") == 0 || strcmp(n->type_name, "IF") == 0) { // if 类似
        Type conditionType = Exp(n->sib->sib);
        if (!(NULL != conditionType && conditionType->kind == BASIC && conditionType->u.basic == 1)) {
            printf("Error type 12 at Line %d: condition type is %d\n", n->sib->sib->lineno, conditionType->kind); 
            assert(0);
        }
        //freeType(conditionType);
        /*Type free_type = */Exp(n->sib->sib->sib->sib);
        //freeType(free_type);
        if (NULL != n->sib->sib->sib->sib->sib) { 
            assert(strcmp(n->sib->sib->sib->sib->sib->type_name, "ELSE") == 0);
            Stmt(n->sib->sib->sib->sib->sib->sib, ntype);
        }
    }
    else assert(0);
}

Type Exp(node_t* node) {
    // TODO: if NULL??
    if (strcmp(node->type_name, "Exp") != 0) assert(0);
    node_t *n = node->fir;
    if (strcmp(n->type_name, "ID") == 0) {
        if (n->sib == NULL) {
            if (checkSymbol(n->id) == 0) {
                printf("Error type 1 at Line %d: undefined variable %s\n", n->lineno, n->id);
                return NULL;
            }
            else {
                return Type_get(n->id); // TODO: 需要copyType吗 
            }
            // TBD: 是否需要检查变量与结构体重名？因为既然定义过了在定义的地方就会检查了
            // 可能是结构体因为 Exp -> Exp DOT ID
        }
        else if (strcmp(n->sib->type_name, "LP") == 0) {
            Type funcType = Type_get(n->id);
            if (funcType == NULL) {
                printf("Error type 2 at Line %d: undefined function %s\n", n->lineno, n->id);
                return NULL;
            }
            if (funcType->kind != FUNCTION) {
                printf("Error type 11 at Line %d: not a function %s\n", n->lineno, n->id);
                return NULL;
            }
            if (strcmp(n->sib->sib->type_name, "Args") == 0) {
                FieldList param = Args(n->sib->sib);
                if (!Param_check(param, funcType->u.function.param)) {
                    printf("Error type 9 at Line %d: incompatible args in function %s\n", n->lineno, n->id);
                    return NULL;
                }
                return funcType->u.function.type;
            }
            else if (strcmp(n->sib->sib->type_name, "RP") == 0) {
                if (!Param_check(NULL, funcType->u.function.param)) {
                    printf("Error type 9 at Line %d: incompatible args in function %s\n", n->lineno, n->id);
                    return NULL;
                }
                return funcType->u.function.type;
            }
            else assert(0);
        }
        else assert(0);
    }
    else if (strcmp(n->type_name, "INT") == 0) {
        Type type = newType(INTType);
        type->assign = RIGHT;
        return type;
    }
    else if (strcmp(n->type_name, "FLOAT") == 0) {
        Type type = newType(FLOATType);
        type->assign = RIGHT;
        return type;
    }
    else if (strcmp(n->type_name, "LP") == 0) {
        return Exp(n->sib);
    }
    else if (strcmp(n->type_name, "MINUS") == 0) {
        //仅有int型和float型变量才能参与算术运算。
        Type expType = Exp(n->sib);
        if (expType == NULL) return NULL;
        if (expType->kind != BASIC) {
            assert(expType->u.basic == 1 || expType->u.basic == 2);
            printf("Error type 7 at Line %d: only INT and FLOAT can do arithmetic calculations (MINUS operator) \n", n->lineno);
            return NULL;
        }
        return expType;
    }
    else if (strcmp(n->type_name, "NOT") == 0) {
        return Exp(n->sib);
    }
    else if (strcmp(n->type_name, "Exp") == 0) {
        Type exp1Type = Exp(n);
        if (NULL == exp1Type) return NULL; // if op1 fails then discard the rest
        if (strcmp(n->sib->sib->type_name, "ID") == 0) {
            // Exp DOT ID
            if (exp1Type->kind != STRUCTURE) {
                printf("Error type 13 at Line %d: not STRUCTURE type\n", n->lineno);
                return NULL;
            }
            Type fType = Type_get_f(exp1Type->u.structure.domain, n->sib->sib->id);
            if (fType == NULL) {
                printf("Error type 14 at Line %d: STRUCTURE has no member %s\n", n->lineno, n->sib->sib->id);
                return NULL;
            }
            return fType;
        }
        Type exp2Type = Exp(n->sib->sib);
        if (NULL == exp2Type) return NULL;
        if (strcmp(n->sib->type_name, "LB") == 0) {
            // Exp -> Exp LB Exp RB
            if (exp1Type->kind != ARRAY) {
                printf("Error type 10 at Line %d: not ARRAY type\n", n->lineno);
                return NULL;
            }
            if (!(exp2Type->kind == BASIC && exp2Type->u.basic == 1)) {
                printf("Error type 12 at Line %d: array query with NONINT type\n", n->lineno);
                return NULL;
            }
            return exp1Type->u.array.elem;
        }
        // 剩下的都是操作符
        if (strcmp(n->sib->type_name, "ASSIGNOP") == 0) {
            if (Type_check(exp1Type, exp2Type) == 0) {
                printf("Error type 5 at Line %d: Type mismatched for assignment.\n", n->lineno);
                return NULL;
            }
            if (exp1Type->assign == RIGHT) {
                printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", n->lineno);
                return NULL;
            }
            return exp1Type;
        }
        else if (strcmp(n->sib->type_name, "PLUS") == 0 || strcmp(n->sib->type_name, "MINUS") == 0 || strcmp(n->sib->type_name, "STAR") == 0 || strcmp(n->sib->type_name, "DIV") == 0 || strcmp(n->sib->type_name, "RELOP") == 0) {
            //仅有int型变量才能进行逻辑运算或者作为if和while语句的条件；仅有int型和float型变量才能参与算术运算。
            if (Type_check(exp1Type, exp2Type) == 0) {
                printf("Error type 7 at Line %d: mismatched types in arithmetic calculations\n", n->lineno);
                return NULL;
            }
            if (exp1Type->kind != BASIC) {
                printf("Error type 7 at Line %d: only INT and FLOAT can do arithmetic calculations\n", n->lineno);
                return NULL;
            }
            if (strcmp(n->sib->type_name, "RELOP") == 0) return newType(INTType); // RELOP
            return exp1Type;
        }
        else if (strcmp(n->sib->type_name, "AND") == 0 || strcmp(n->sib->type_name, "OR") == 0) {
            if (Type_check(exp1Type, exp2Type) == 0) {
                printf("Error type 7 at Line %d: mismatched types in logical calculations\n", n->lineno);
                return NULL;
            }
            if (!(exp1Type->kind == BASIC && exp1Type->u.basic == 1)) {
                printf("Error type 7 at Line %d: only INT can do logical calculations\n", n->lineno);
                return NULL;
            }
            return exp1Type;
        }
        else assert(0);
    }
    else assert(0);
    assert(0);
}

FieldList Args(node_t* node) {
    // Args -> Exp COMMA Args | Exp
    node_t* n = node->fir;
    assert(strcmp(n->type_name, "Exp") == 0);
    Type type = Exp(n);
    FieldList f = (FieldList)malloc(sizeof(struct FieldList_));
    f->type = type;
    f->name = NULL;
    if (NULL == n->sib) {
        f->tail = NULL;
    }
    else f->tail = Args(n->sib->sib);
    return f;
}

FieldList VarDec(node_t* node, Type type, int structflag) { // 为什么返回FieldList
    // VarDec -> ID | VarDec LB INT RB
    node_t *n = node->fir;
    if (strcmp(n->type_name, "ID") == 0) {
        if (checkField(n->id) == 1) {
            if (structflag == 0) printf("Error type 3 at Line %d: redefined variable\n", n->lineno);
            else printf("Error type 15 at Line %d: redefine member in structure field\n", n->lineno);
        }
        else {
            insertSymbol(n->id, type); /*TODO: n的符号名及类型插入全局符号表中;*/
        }
        FieldList field = (FieldList)malloc(sizeof(struct FieldList_));
        field->name = n->id;
        field->tail = NULL;
        field->type = type;
        return field; // 出错/不是struct也先返回好了，应该不会插入，防止中断(但是涉及内存释放问题) ||||| 思考：field list重复怎么办？  插入好像也没问题，除非两个类型不一致，暂时不管，可在DecList改
    }
    else if (strcmp(n->type_name, "VarDec") == 0) {
        Type arrType = newType(ArrayType);
        assert(strcmp(n->sib->sib->type_name, "INT") == 0);
        arrType->u.array.size = n->sib->sib->num.type_int;
        arrType->u.array.elem = type;
        FieldList f = VarDec(n, arrType, structflag);
        return f;
    }
    else assert(0);
}

FieldList Dec(node_t* node, Type type, int structflag) {
    node_t* n = node->fir;
    // Dec -> VarDec | VarDec ASSIGNOP Exp
    FieldList f = VarDec(n, type, structflag); // 是否应该传type？type是什么？
    if (NULL == n->sib) {
        return f;/*insertField(f)，需判断是否重复*/
    }
    else {
        assert(strcmp(n->sib->sib->type_name, "Exp") == 0);
        if (structflag) {
            printf("Error type 15 at Line %d: structure field type cannot be initialized\n", n->lineno);
        }
        else {
            Type expType = Exp(n->sib->sib);
            if (NULL == expType) return NULL;
            if (Type_check(type, expType) == 0) {
                printf("Error type 5 at Line %d: assign mismatched Type\n", n->lineno);
                // return NULL; // modified by Zn
                // return NULL; // modified by Zn
                // return NULL; // modified by Zn
            }
        }
        return f; /*insertField(f)，still try to insert the field*/
    }
}

FieldList DecList(node_t* node, Type type, int structflag) {
    assert(strcmp(node->type_name, "DecList") == 0);
    node_t* n = node->fir;
    FieldList f = Dec(n, type, structflag);
    if (NULL != n->sib) f->tail = DecList(n->sib->sib, type, structflag);
    else f->tail = NULL;
    return f;
}

Function FunDec(node_t* node, Type type){
    // Func -> ID LP VarList RP | ID LP RP
    assert(node);
    node_t* n = node->fir;
    if (checkField(n->id) != 0) { // 还是用check_func??
        printf("Error type 4 at Line %d: redefined function\n", n->lineno);
        return NULL;
    }
    Function func = (Function)malloc(sizeof(struct Function_));
    strcpy(func->name, n->id);
    func->line = n->lineno;
    func->type = type;
    Type funcType = newType(FuncType);
    funcType->u.function = *func; // 小心被delete，指向相同的地址了，很危险！！！！！
    insertSymbol(n->id, funcType);
    newScope(); //<----------------------------------------------------------------
    if (strcmp(n->sib->sib->type_name, "VarList") == 0) {
        func->param = VarList(n->sib->sib); 
        return func;
    }
    else if (strcmp(n->sib->sib->type_name, "RP") == 0) {
        func->param = NULL;
        return func;
    }
    else assert(0);
}

FieldList VarList(node_t* node) {
    assert(strcmp(node->type_name, "VarList") == 0);
    node_t* n = node->fir;
    FieldList f = ParamDec(n);
    if (NULL != n->sib) f->tail = VarList(n->sib->sib);
    else f->tail = NULL;
    return f;
}

FieldList ParamDec(node_t* node) { //仔细check
    node_t* n = node->fir;
    // ParamDec -> Specifier VarDec
    Type specType = Specifier(n);
    FieldList f = VarDec(n->sib, specType, 0); /*NULL == specType照样插入，不知道有没有问题*/
    return f;
}