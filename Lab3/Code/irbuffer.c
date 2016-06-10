#include "symbols.h"
#include "irbuffer.h"
#include <assert.h>

static int labelNum=0,tempNum=0;
codeNode *headNode=NULL;
//int to string,code by internet
void itoa(unsigned long val, char* buf,unsigned radix){
	char *p;/* pointer to traverse string */
	char *firstdig;/* pointer to first digit */
	char temp;/* temp char */
	unsigned digval;/* value of digit */
	p = buf;
	firstdig = p;/* save pointer to first digit */
	do {
		digval = (unsigned)(val % radix);
		val /= radix;/* get next digit */
		/* convert to ascii and store */
		if (digval > 9)
			*p++ = (char)(digval - 10 + 'a');/* a letter */
		else
			*p++ = (char)(digval + '0');/* a digit */
	} while (val > 0);
	/* We now have the digit of the number in the buffer, but in reverse
	 * order. Thus we reverse them now. */
	*p-- = '\0';/* terminate string; p points to last digit */
	do {
		temp = *p;
		*p = *firstdig;
		*firstdig = temp;		/* swap *p and *firstdig */
		--p;
		++firstdig;				/* advance to next two digits */
	} while (firstdig < p);		/* repeat until halfway */
}

void newLabel(char *name){
	strcpy(name,"label");
	char t[16];
	itoa(labelNum,t,10);
	labelNum++;
	strcat(name,t);
}
void newTemp(char *name){
	strcpy(name,"_t");
	char t[16];
	itoa(tempNum,t,10);
	tempNum++;
	strcat(name,t);
}
void addCode(int argsNum,...){
	codeNode *p=(codeNode*)malloc(sizeof(codeNode));
	p->argsNum=argsNum;
	va_list argslist;
	va_start(argslist,argsNum);
	for(int i=0;i<argsNum;i++){
		strcpy(p->args[i],va_arg(argslist,char*));
	}
	if(headNode==NULL){
		p->pre=p;
		p->next=p;
		headNode=p;
	}
	else{
		p->next=headNode;
		p->pre=headNode->pre;//headNode->pre is the one at the end
		headNode->pre->next=p;
		headNode->pre=p;
	}
}
void printLine(FILE *fp,codeNode *p){
	fprintf(fp,"%s",p->args[0]);
	for(int i=1;i<p->argsNum;i++)
		fprintf(fp," %s",p->args[i]);
	fprintf(fp,"\n");
}
void printCode(char *file){
	if(headNode==NULL)return;
	FILE *fp;
	fp=fopen(file,"w");
	assert(fp!=NULL);
	codeNode *p=headNode;
	do{
		printLine(fp,p);
		p=p->next;
	}while(p!=headNode);
	fclose(fp);
}
