#include "symbol.h"
#include <assert.h>

HashNode *gTable[HASH_TABLE_SIZE + 2];
HashNode *sTable[HASH_TABLE_SIZE + 2];

void initTable() {
    // 默认为NULL吗
}

Type newType(enum TypeID typeId) {
    //仅有int型和float型变量才能参与算术运算?? 这不是废话吗
    //o.f = 1不允许吧？？
    Type type = (Type)malloc(sizeof(struct Type_));
    switch (typeId) {
        case INTType:
            type->kind = BASIC;
            type->u.basic = 1;
            break;
        
        case FLOATType:
            type->kind = BASIC;
            type->u.basic = 2;
            break;

        case ArrayType:
            assert(0);
            // type->kind = ARRAY;
            // type->u.array.elem = NULL;
            // type->u.array.size = -1;
            // type->assign = BOTH;
            break;
        
        case StructType:
            assert(0);
            type->kind = STRUCTURE;
            type->u.structure.domain = NULL;
            type->assign = BOTH; // Structure里面的也算structure吗，不算吧, BOTH!!!!!
            Type sType = (Type)malloc(sizeof(struct Type_));
            break;
        
        case FuncType:
            assert(0);
            // type->kind = FUNCTION;
            // type->u.function = NULL;
            // type->assign = RIGHT;
            /*函数返回值类型是right*/
            break;
            
        default:
            assert(0);
            break;
    }
}

// void freeType(Type type) {
//     if (!type) return;
//     assert(0);
// }

int Type_check(Type t1, Type t2) {
    if (t1 == NULL && t2 == NULL)
        return 1; // 都为空一致
    if (t1->kind == BASIC)
        if (t1->u.basic == t2->u.basic)
            return 1; // 基本类型相同
        return 0;     // 基本类型不同
    if (t1->kind == ARRAY) // 逐个比对数组元素类型
        return Type_check(t1->u.array.elem, t2->u.array.elem);
    if (t1->kind == STRUCTURE)
        if (t2->kind != STRUCTURE)
            return 0;
        return !strcmp(t1->u.structure.name, t2->u.structure.name);
}

int Param_check(FieldList p1, FieldList p2) {
    while (p1 != NULL && p2 != NULL) {
        if (Type_check(p1->type, p2->type) == 0)
            return 0; // 参数类型不一致
        p1 = p1->tail;
        p2 = p2->tail; // 获取下一个参数
    }
    if (p1 == NULL && p2 == NULL)
        return 1;   // 检查结束，参数类型一致  
    return 0;       // 默认类型不一致
}

//--------------------------------------

int insertSymbol(char *name, Type type){
    unsigned int hash = hashFunc(name);
    HashNode hashNode = (HashNode) malloc(sizeof(struct HashNode_));
    hashNode->name = name; hashNode->type = type;
    // 构建新的符号并用name、type初始化;
    // HashNode *node = gTable[hash];
    // if(node==NULL){
    //     gTable[hash] = newnode;
    //     return 0;
    // } else {
    //     循环遍历直到node->next为NULL，
    //     插入；
    // }
}

int checkSymbol(char *name){
    unsigned int hash = hashFunc(name);
    // for(HashNode *node = gTable[hash]; node!=NULL; node = node->next){
    //     if(strcmp(node->name, name) == 0){
    //         return 1;
    //     }
    // }
    // return 0;
}

Type Type_get(char *name) {

}

Type Type_get_f(FieldList domain, char *name) {
    if (NULL == domain) return NULL;
    assert(NULL != domain->name);
    if (strcmp(domain->name, name) == 0) return domain->type;
    return Type_get_f(domain->tail, name);
}
