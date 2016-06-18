#ifndef TREE_H
#define TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define MAX_NUM_OF_CHILD 16

typedef enum{T_OTHER,T_INT,T_FLOAT,T_ID,NT}NodeKind;
//For Lab2
typedef enum TYPES{
	/*Tokens*/
	None,ID_,INT_,FLOAT_,SEMI_,COMMA_,ASSIGNOP_,RELOP_,PLUS_,MINUS_,STAR_,DIV_,
	AND_,OR_,DOT_,NOT_,TYPE_,LP_,RP_,LB_,RB_,LC_,RC_,STRUCT_,RETURN_,IF_,ELSE_,WHILE_,
	/*High-level Definitions*/
	Program,ExtDefList,ExtDef,ExtDecList,
	/*Specifiers*/
	Specifier,StructSpecifier,OptTag,Tag,
	/*Declarators*/
	VarDec,FunDec,VarList,ParamDec,
	/*Statements*/
	CompSt,StmtList,Stmt,
	/*Local Definitions*/
	DefList,Def,DecList,Dec,
	/*Expressions*/
	Exp,Args
}TYPES;

struct TreeNode{
	NodeKind type;
	TYPES T;
	char nodeName[32];
	char text[64];
	int lineNum;
	int childNum;
	//struct TreeNode* parent;
	struct TreeNode* child[MAX_NUM_OF_CHILD];
};

typedef struct TreeNode Node;

Node* addNode(char *name,TYPES TP,int num,int line, ...);

void printTree(Node *p,int depth);

#endif
