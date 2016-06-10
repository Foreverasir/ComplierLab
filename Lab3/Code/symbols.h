#ifndef _SYMBOLS_H_
#define _SYMBOLS_H_

#include <stdlib.h>
#include <string.h>
#include "Tree.h"

typedef enum valueKind{INT, FLOAT, USERDEF}valueKind;
typedef enum typeKind{ARRAY, STRUCTURE}typeKind;//user define

struct Type_;
struct Val_;

typedef struct arrayDefList{
	int depth;//dimension
	int number;
	int eleSize;
	valueKind kind;
	struct Type_ *valType;
	struct arrayDefList *next;
}arrayDefList;
typedef struct structDefList{
	int defCount;
	struct Val_ **defList;
}structDefList;

typedef struct Type_{
	char name[64];
	typeKind kind;
	union{arrayDefList *adl;structDefList *sdl;}def;
	struct Type_ *next;
}Type_;
typedef struct Val_{
	char name[64];
	valueKind kind;
	struct Val_ *next;
	struct Type_ *valType;
	int basicFlag;//0:a field 1:can be used directly 2:it is a struct
	//int cmpFlag;//0:not used 1:compared
}Val_;
typedef struct Func_{
	char name[64];
	int paraCount;
	struct Func_ *next;
	valueKind *kinds;
	valueKind returnKind;
	Type_ *returnType;
	Type_ **para;
	Val_ **paraList;
}Func_;

/*Table define*/
typedef struct Symbols{
	Type_ *typeTable;
	Val_ *valueTable;
	Func_ *funcTable;
}Symbols;

void initSymbols();
/*Create*/
Type_* newType(const char* name);//define type
Val_* newValue(const char* name);//define value
Func_* newFunc(const char* name);//define function
/*Search fail return NULL*/
Type_* getType(const char* name);
Val_* getValue(const char* name);
Func_* getFunc(const char* name);
/*Add to table*/
Type_* addType(Type_ *t);
Val_* addValue(Val_ *v);
Func_* addFunc(Func_ *f);

int typeEqual(Type_ *t1,Type_ *t2);

void getName(char* name);//help no name struct

/*Handle Array*/
void arrayInit(Type_ *t,valueKind kind,Type_* valueType,int num);
void arrayGrow(Type_ *t,int num);

int getStructSize(Type_ *t);
int getOffset(Type_ *t,char *field);
#endif
