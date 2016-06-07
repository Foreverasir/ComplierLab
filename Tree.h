#ifndef TREE_H
#define TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define MAX_NUM_OF_CHILD 16

typedef enum{T_OTHER,T_INT,T_FLOAT,T_ID,NT}NodeKind;

struct TreeNode{
	NodeKind type;
	char nodeName[32];
	char text[32];
	int lineNum;
	int childNum;
	struct TreeNode* child[MAX_NUM_OF_CHILD];
};

typedef struct TreeNode Node;

Node* addNode(char *name,int num,int line, ...);

void printTree(Node *p,int depth);

#endif
