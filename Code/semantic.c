#include "semantic.h"

void semantic_check(node_t* root) {
    /* init */
    ExtDefList(root->fir);
    /* memory management */
}

void ExtDefList(node_t* node) {
    if (NULL == node) return;
    ExtDef(node->fir);
    ExtDefList(node->fir->sib);
}

void ExtDef(node_t* node) {
    assert(NULL != node);  //if (NULL == node) return;
    // ExtDef -> Specifier ExtDecList SEMI
    // ExtDef -> Specifier SEMI
    // ExtDef -> Specifier FunDec CompSt
    node_t* spec = node->fir, * n = spec->sib;
    if (strcmp(n->type_name, "SEMI") == 0) {
        ExtSpecifier(spec);
    }
    else if (strcmp(n->type_name, "FunDec") == 0) {
        /**/
    }
    else if (strcmp(n->type_name, "ExtDecList") == 0) {
        Type varType = Specifier(spec);
        if (NULL != varType) ExtDec(n, varType);
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

void ExtSpecifier(node_t* node) {
    // ExtDef -> Specifier SEMI
    node_t* n = node->fir;
    if (strcmp(n->type_name, "TYPE") == 0) {
        return;
    }
    else if (strcmp(n->type_name, "StructSpecifier") == 0) {
        node_t* child = n->fir; assert(child);
        if (strcmp(child->sib->type_name, "OptTag") == 0) {
            /*TODO*/ //!!!!maybe NULL
        }
        else if (strcmp(child->sib->type_name, "Tag") == 0) {
            return;// 声明不会重复，而且可以存在空定义
        }
    }
    else assert(0);
}

Type Specifier(node_t* node) {
    // Specifier -> TYPE | StructSpecifier
    // StructSpecifier -> STRUCT OptTag LC DefList RC | STRUCT Tag
    node_t* n = node->fir; assert(n);
    if (strcmp(n->type_name, "TYPE") == 0) {
        if (n->type == TOKEN_INT) return newType(INTType);
        else if (n->type == TOKEN_FLOAT) return newType(FLOATType);
        else assert(0);
    }
    else if (strcmp(n->type_name, "StructSpecifier") == 0) {
        StructSpecifier(n);
        // node_t* child = n->fir; assert(child);
        // if (strcmp(child->sib->type_name, "OptTag") == 0) {
        //     //
        // }
        // else if (strcmp(child->sib->type_name, "Tag") == 0) {
        //     //Type type = Type_get(child->sib->id); //!!!!!!!!!!!!!!!!!!!!!

        // }
        // else assert(0);
    }
    else assert(0);
}

Type StructSpecifier(node_t* node) {
    // StructSpecifier -> STRUCT OptTag LC DefList RC | STRUCT TAG
    node_t *n = node->fir; assert(n);
    if (n->sib->sib == NULL) { // STRUCT TAG, 声明 
        assert(strcmp(n->sib->type_name, "Tag") == 0);
        //
    }
    else if (strcmp(n->sib->type_name, "OptTag") == 0) {
        Type sType = newType(StructType);
        strcpy(sType->u.structure.name, n->fir->fir->id);
        if (checkSymbol(sType->u.structure.name)) {
            printf("Error type 16 at Line %d: %s\n", n->fir->fir->lineno, sType->u.structure.name); // 结构体重名
            return NULL;
        }
        insertSymbol(sType->u.structure.name, sType);
        //sType->u.structure.domain = NULL; // TODO
        
        //     DefList(n->sib->sib->sib); // DefList(n的子节点, 1)
        //     插入符号表中;
        //     返回sType
    }
    else if (strcmp(n->sib->type_name, "LC") == 0) { // 匿名结构体
    //     与一般结构体定义类似，但名称可以定义为随机值或者置为NULL;
    //     插入符号表并返回结构体类型sType;
    }
    else assert(0);
}


void CompSt(node_t* node, Type ntype) {
    // CompSt -> LC DefList StmtList RC
    /*newScope()*/
    if (NULL == node) return;
    assert (strcmp(node->fir->sib->type_name, "DefList") == 0);
    DefList(node->fir->sib, NULL);
    assert (strcmp(node->fir->sib->sib->type_name, "StmtList") == 0);
    StmtList(node->fir->sib->sib, ntype);
    /*deleteScope()*/
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
        if (expType && Type_check(expType, ntype) == 0) {
            printf("Error type 8 at Line %d: error return type %d\n", n->lineno, expType->kind); // 返回值错误
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
    // TODO: if NULL??arithmeticarithmeticarithmetic
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
                printf("Error type 5 at Line %d: ASSIGN must have same type\n", n->lineno);
                return NULL;
            }
            if (exp1Type->assign == RIGHT) {
                printf("Error type 6 at Line %d: ASSIGN applied to non LEFT value\n", n->lineno);
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

FieldList VarDec(node_t* node, Type type, int structflag) { // 为什么返回FieldList
    // VarDec -> ID | VarDec LB INT RB
    node_t *n = node->fir;
    if (strcmp(n->type_name, "ID") == 0) {
        if (structflag == 0 && checkSymbol(n->id) == 1) { // 为什么不是结构体  struct tag a; ???????????????
            printf("Error type 3 at Line %d: redefined variable\n", n->lineno); 
        }
        FieldList field = (FieldList)malloc(sizeof(struct FieldList_));
        strcpy(field->name, n->id);
        field->tail = NULL; // ??
        field->type = type;
        insertSymbol(n->id, type); /*TODO: n的符号名及类型插入全局符号表中;*/
        return field;
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

Function FunDec(node_t* node, Type type){
    // Func -> ID LP VarList RP | ID LP RP
    assert(node);
    node_t* n = node->fir;
    if (checkSymbol(n->id) != 0) { // 还是用check_func??
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
    /* newScope */
    if (strcmp(n->sib->sib->type_name, "VarList") == 0) {
        func->param = VarList(n); 
        return func;
    }
    else if (strcmp(n->sib->sib->type_name, "RP") == 0) {
        func->param = NULL;
        return func;
    }
    else assert(0);
}

FieldList VarList(node_t* node) {
    node_t* n = node->fir;
    FieldList f = (FieldList)malloc(sizeof(struct FieldList_));
    /*TODO*/
}

FieldList Dec(node_t* node, Type type, int structflag){
    node_t* n = node->fir;
    if (NULL != n->sib) {
        assert(strcmp(n->sib->sib->type_name, "Exp") == 0);
        Type expType = Exp(n->sib->sib);
        if (Type_check(type, expType) == 0) {
            printf("Error type 5 at Line %d: assign mismatched Type\n", n->lineno);
            return NULL;
        }
    }
}