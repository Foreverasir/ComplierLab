#ifndef _IR_BUFFER_
#define _IR_BUFFER_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct codeNode{
	int argsNum;
	char args[6][32];
	struct codeNode* pre;
	struct codeNode* next;
}codeNode;

void newLabel(char *name);
void newTemp(char *name);
void addCode(int argsNum,...);
void printCode(char* file);
void itoa(unsigned long val, char* buf,unsigned radix);
#endif
