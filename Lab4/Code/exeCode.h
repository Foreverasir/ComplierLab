#include "irbuffer.h"

typedef struct VariableLocation{
	char name[32];
	int loc;
}VarL;

void generateCode(codeNode *h,FILE *fp);
