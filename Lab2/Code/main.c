#include <stdio.h>
#include "syntax.tab.h"
#include "Tree.h"
#include "semantics.h"
extern int yylineno;
extern Node *root;
extern int isError;
extern void yyrestart();
int main(int argc, char** argv){
	if (argc <= 1) return 1;
	FILE* f = fopen(argv[1], "r");
	if (!f){
		perror(argv[1]);
		return 1;
	}
	yyrestart(f);
	if(yyparse()==0 && isError==0){
		//printTree(root,0);
		initSymbols();
		staInit();
		semanticMain(root);
	}
	return 0;
}
