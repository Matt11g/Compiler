#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct Structure_* Structure;
typedef struct Function_* Function;

enum TypeID {INTType, FLOATType, ArrayType, StructType, FuncType/*, StructContainerType*/};

struct Type_{
    enum {
        BASIC,     // variable
        ARRAY,     // array
        STRUCTURE, // structure
        FUNCTION,   // function
        //STRUCT_CONTAINER
    } kind;
    union{
        int basic; // int(1), float(2)
        struct {
            Type elem; int size;
        } array; 
        // 结构体类型，包含名称、域
        struct Structure_{
            char name[32 + 2];
            FieldList domain;
        } structure;
        // 函数类型，包括名称、行号、返回值类型、参数
        struct Function_{
            char name[32 + 2];
            int line;
            Type type;
            FieldList param;
        }* function;
    } u;
    enum {
        LEFT,    // left value
        RIGHT,   // right value
        BOTH     // left | right value
    } assign;
};

// 变量、参数、结构体的域
struct FieldList_{
    char *name;
    Type type;
    FieldList tail;
};
// 结构体类型，包含名称、域
// struct Structure_{
//     char *name;
//     FieldList domain;
// };
// 函数类型，包括名称、行号、返回值类型、参数
// struct Function_{
//     char *name;
//     int line;
//     Type type;
//     FieldList param;
// };

Type newType(enum TypeID typeId);
//void freeType(Type type);

int Type_check(Type t1,Type t2);
int Param_check(FieldList p1,FieldList p2);

//-----------------------------------------------

// open hashing
typedef struct HashNode_* HashNode;
struct HashNode_{
    char *name;
    Type type;
    //FieldList param;
    struct HashNode_ *next;
    struct HashNode_ *nxt; // in scope
};

#define HASH_TABLE_SIZE 0x3fff
extern HashNode gTable[HASH_TABLE_SIZE + 2];
//extern HashNode sTable[HASH_TABLE_SIZE + 2];

typedef struct _symtab {
    struct _symtab* par;
    HashNode table;
} symtab;

symtab* newScope();
symtab* deleteScope();
//void freeMap(HashNode node);

extern symtab* head;

void initTable();

static inline unsigned int hashFunc(char *key) {
    unsigned int val = 0, i;
    for (; *key; ++key) {
        val = (val << 2) + *key;
        if (i = val & ~HASH_TABLE_SIZE) val = (val ^ (i >> 12)) & HASH_TABLE_SIZE;
    }
    return val;
}

void insertSymbol(char *name, Type type);
int checkSymbol(char *name);
int checkField(char *name);
Type Type_get(char *name);
Type Type_get_f(FieldList domain, char *name);

#endif