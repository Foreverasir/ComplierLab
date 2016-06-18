#include "Tree.h"
Node* addNode(char *name,TYPES TP,int num,int line, ...){
	va_list vaList;//get unsure parameters
	int i=0;
	Node *current=malloc(sizeof(Node));
	Node *temp;

	current->type=NT;
	current->T=TP;
	strcpy(current->nodeName,name);
	current->lineNum=line;
	current->childNum=0;

	va_start(vaList,line);
	for(;i<num;i++){
		temp=va_arg(vaList,Node*);
		current->child[i]=temp;
		current->childNum++;
		/*if(temp!=NULL){
			current->child[i]=temp;
			current->childNum++;
		}
		else current->childNum++;*/
	}
	va_end(vaList);
	return current;
}

void printTree(Node *p,int depth){
	int i=0;
	if(p==NULL)return;
	if(p->T==None)return;
	for(;i<depth;i++)printf("  ");
	switch(p->type){
		case T_OTHER:	
			if(strcmp(p->nodeName,"TYPE")==0)
				printf("%s: %s\n",p->nodeName,p->text);
			else printf("%s\n",p->nodeName);break;
		case T_INT:		printf("%s: %d\n",p->nodeName,atoi(p->text));break;
		case T_FLOAT:	printf("%s: %f\n",p->nodeName,atof(p->text));break;
		case T_ID:		printf("%s: %s\n",p->nodeName,p->text);break;
		case NT:		printf("%s (%d)\n",p->nodeName,p->lineNum);
						for(i=0;i<p->childNum;i++)
							printTree(p->child[i],depth+1);
						break;
		default:		printf("Error NodeKind!\n");break;
	}
}
