#include "exeCode.h"

#define MAX_NUM 1024

FILE *f;
static codeNode* headNode;
static int currentArgsNum;//args of function
static int currentSize;//the size of used stack for Func.
static int varCount;
static VarL vtable[MAX_NUM];
static int specialflag=0;
void initSta(){
	currentArgsNum=0;
	currentSize=0;
	varCount=0;
}

void init(FILE *f);

/*Get a VarL from vtable*/
int getVarL(char *name){
	if(name[0]=='#') return -1;//it's an immediate value
	if(name[0]=='*' || name[0]=='&')
		name++;
	for(int i=0;i<varCount;i++)
		if(strcmp(vtable[i].name,name)==0)
			return vtable[i].loc;
	return -1;//not found
}

/*Add a new VarL to vtable*/
void addVarL(char* name,int size0){
	if(name[0]=='#') return;
	if(name[0]=='*' || name[0]=='&')
		name++;
	if(getVarL(name)!=-1) return;
	vtable[varCount].loc=currentSize;
	currentSize += size0;
	strcpy(vtable[varCount].name,name);
	varCount++;
	if(varCount>=MAX_NUM){
		printf("Reached max vtable size.\n");
		exit(0);
	}
}

/**/
void prepareReg(int num,char* name){
	char t[8];
	sprintf(t,"$t%d",num);
	switch(name[0]){
		case '#':
			fprintf(f,"li %s, %s\n",t,&name[1]);break;
		case '*':
			fprintf(f,"lw %s, %d($sp)\n",t,getVarL(name));
			fprintf(f,"add %s, %s, $sp\n",t,t);
			fprintf(f,"lw %s, 0(%s)\n",t,t);break;	
		case '&':
			fprintf(f,"li %s, %d\n",t,getVarL(name));break;
		default:
			fprintf(f,"lw %s, %d($sp)\n",t,getVarL(name));break;
	}
}

/*generate VarL for each function*/
void generateVarL(codeNode *p){
	switch(p->argsNum){
		case 2://except GOTO 
			if(strcmp(p->args[0],"GOTO")!=0)
				addVarL(p->args[1],4);
			break;
		case 3:
			if(strcmp(p->args[1],":=")==0){
				addVarL(p->args[0],4);
				addVarL(p->args[2],4);
			}
			else if(strcmp(p->args[0],"DEC")==0)
				addVarL(p->args[1],strtol(p->args[2],NULL,10));
			break;
		case 4://x := CALL f
			addVarL(p->args[0],4);break;
		case 5:
			addVarL(p->args[0],4);
			addVarL(p->args[2],4);
			addVarL(p->args[4],4);
			break;
		case 6://IF x relop y GOTO z
			addVarL(p->args[1],4);
			addVarL(p->args[3],4);
			break;
		default:printf("Ir Code exception.\n");break;
	}
}

/*generate Code for each ir-line*/
void generateLine(codeNode *p){
	switch(p->argsNum){
		case 2://Attention LABEL x : is 3
			if(strcmp("GOTO",p->args[0])==0)
				fprintf(f,"j %s\n",p->args[1]);
			else if(strcmp("RETURN",p->args[0])==0){
				prepareReg(0,p->args[1]);//use t0
				fprintf(f,"move $v0, $t0\n");
				fprintf(f,"addi $sp, $sp, %d\n",currentSize);
				fprintf(f,"jr $ra\n");
			}else if(strcmp("ARG",p->args[0])==0){
				prepareReg(0,p->args[1]);
				if(currentArgsNum<4)
					fprintf(f,"move $a%d, $t0\n",currentArgsNum);
				else if(currentArgsNum<12)
					fprintf(f,"move $s%d, $t0\n",currentArgsNum-4);
				else if(currentArgsNum<20)
					fprintf(f,"move $t%d, $t0\n",currentArgsNum-10);	
				currentArgsNum++;
			}else if(strcmp("PARAM",p->args[0])==0){
				int index;
				codeNode *current=p;
				for(index=0;strcmp("PARAM",current->next->args[0])==0;current=current->next,index++);
				if(specialflag==0 && index>=20){
					specialflag=1;
					fprintf(f,"move $t0, $fp\n");
				}
				if(specialflag==1 && index<20){
					fprintf(f,"move $fp, $t0\n");
					specialflag=0;
				}
				if(index<4)
					fprintf(f,"sw $a%d, %d($sp)\n",index,getVarL(p->args[1]));
				else if(index<12)
					fprintf(f,"sw $s%d, %d($sp)\n",index-4,getVarL(p->args[1]));
				else if(index<20)
					fprintf(f,"sw $t%d, %d($sp)\n",index-10,getVarL(p->args[1]));
				else{
					fprintf(f,"addi $fp, $fp, 4\n");
					fprintf(f,"lw $t1, 0($fp)\n");
					fprintf(f,"sw $t1, %d($sp)\n",getVarL(p->args[1]));
				}
			}else if(strcmp("READ",p->args[0])==0){
				fprintf(f,"addi $sp, $sp, -4\n");
				fprintf(f,"sw $ra, 0($sp)\n");
				fprintf(f,"jal read\n");
				fprintf(f,"lw $ra, 0($sp)\n");
				fprintf(f,"addi $sp, $sp, 4\n");
				if(p->args[1][0]=='*'){	
					fprintf(f,"lw $t0, %d($sp)\n",getVarL(p->args[1]));
					fprintf(f,"add $t0, $t0, $sp\n");
					fprintf(f,"sw $v0, 0($t0)\n");
				}
				else
					fprintf(f,"sw $v0, %d($sp)\n",getVarL(p->args[1]));
			}else if(strcmp("WRITE",p->args[0])==0){
				prepareReg(0,p->args[1]);
				fprintf(f,"move $a0, $t0\n");
				fprintf(f,"addi $sp, $sp, -4\n");
				fprintf(f,"sw $ra, 0($sp)\n");
				fprintf(f,"jal write\n");
				fprintf(f,"lw $ra, 0($sp)\n");
				fprintf(f,"addi $sp, $sp, 4\n");
			}break;
		case 3:
			if(strcmp(":=",p->args[1])==0){
				prepareReg(0,p->args[2]);
				if(p->args[0][0]=='*'){
					fprintf(f,"lw $t1, %d($sp)\n",getVarL(p->args[0]));
					fprintf(f,"add $t1, $t1, $sp\n");
					fprintf(f,"sw $t0, 0($t1)\n");
				}else
					fprintf(f,"sw $t0, %d($sp)\n",getVarL(p->args[0]));
			}else if(strcmp("DEC",p->args[0])!=0)//LABEL
				fprintf(f,"%s:\n",p->args[1]);
			break;
		case 4://x := CALL f
			currentArgsNum=0;
			fprintf(f,"addi $sp, $sp, -4\n");
			fprintf(f,"sw $fp, 0($sp)\n");
			fprintf(f,"move $fp, $sp\n");
			fprintf(f,"addi $sp, $sp, -4\n");
			fprintf(f,"sw $ra, 0($sp)\n");
			if(strcmp("main",p->args[3])==0)
				fprintf(f,"jal %s\n",p->args[3]);
			else
				fprintf(f,"jal _%s\n",p->args[3]);
			fprintf(f,"lw $ra, 0($sp)\n");
			fprintf(f,"addi $sp, $sp, 4\n");
			fprintf(f,"lw $fp, 0($sp)\n");
			fprintf(f,"addi $sp, $sp, 4\n");
			if(p->args[0][0]=='*'){
				fprintf(f,"lw $t0, %d($sp)\n",getVarL(p->args[0]));
				fprintf(f,"add $t0, $t0, $sp\n");
				fprintf(f,"sw $v0, 0($t0)\n");	
			}
			else
				fprintf(f,"sw $v0, %d($sp)\n",getVarL(p->args[0]));
			break;
		case 5:
			prepareReg(0,p->args[2]);
			prepareReg(1,p->args[4]);
			switch(p->args[3][0]){
				case '+':fprintf(f,"add $t0, $t0, $t1\n");break;
				case '-':fprintf(f,"sub $t0, $t0, $t1\n");break;
				case '*':fprintf(f,"mul $t0, $t0, $t1\n");break;
				case '/':fprintf(f,"div $t0, $t1\n");fprintf(f,"mflo $t0\n");break;
				default:printf("Illegal Oprand.\n");break;
			}
			if(p->args[0][0]=='*'){
				fprintf(f,"lw $t1, %d($sp)\n",getVarL(p->args[0]));
				fprintf(f,"add $t1, $t1, $sp\n");
				fprintf(f,"sw $t0, 0($t1)\n");	
			}
			else
				fprintf(f,"sw $t0, %d($sp)\n",getVarL(p->args[0]));
			break;
		case 6:
			prepareReg(0,p->args[1]);
			prepareReg(1,p->args[3]);
			if(strcmp("==",p->args[2])==0)
				fprintf(f,"beq $t0, $t1, %s\n",p->args[5]);
			else if(strcmp("!=",p->args[2])==0)
				fprintf(f,"bne $t0, $t1, %s\n",p->args[5]);
			else if(strcmp(">",p->args[2])==0)
				fprintf(f,"bgt $t0, $t1, %s\n",p->args[5]);
			else if(strcmp("<",p->args[2])==0)
				fprintf(f,"blt $t0, $t1, %s\n",p->args[5]);
			else if(strcmp(">=",p->args[2])==0)
				fprintf(f,"bge $t0, $t1, %s\n",p->args[5]);
			else if(strcmp("<=",p->args[2])==0)
				fprintf(f,"ble $t0, $t1, %s\n",p->args[5]);
			break;
		default:printf("Illegal code.\n");break;
	}
}

/*generate target Code of one function*/
void generateFunc(codeNode *start,codeNode *end){
	initSta();
	if(strcmp("main",start->args[1])==0)
		fprintf(f,"\n%s:\n",start->args[1]);//func name
	else
		fprintf(f,"\n_%s:\n",start->args[1]);//func name
	codeNode *p=start->next;
	while(p!=end){
		generateVarL(p);
		p=p->next;
	}
	p=start->next;
	fprintf(f,"addi $sp, $sp, -%d\n",currentSize);//stack grows from high to low address
	while(p!=end){
		generateLine(p);
		p=p->next;
	}
}

/*Called from printCode*/
void generateCode(codeNode *h,FILE *fp){
	initSta();
	f=fp;
	headNode=h;
	init(f);
	codeNode *p=headNode;
	codeNode *q=NULL;
	do{
		q=p->next;
		while(1){
			if(strcmp(q->args[0],"FUNCTION")==0)break;
			else if(q==headNode)break;
			q=q->next;
		}
		generateFunc(p,q);
		p=q;
	}while(p!=headNode);
}

void init(FILE *f){
	fprintf(f,"%s\n",".data");
	fprintf(f,"%s\n","_prompt: .asciiz \"Enter an integer:\"");
	fprintf(f,"%s\n","_ret: .asciiz \"\\n\"");
	fprintf(f,"%s\n",".globl main");
	fprintf(f,"%s\n",".text");
	fprintf(f,"%s\n","read:");
	fprintf(f,"%s\n","li $v0, 4");
	fprintf(f,"%s\n","la $a0, _prompt");
	fprintf(f,"%s\n","syscall");
	fprintf(f,"%s\n","li $v0, 5");
	fprintf(f,"%s\n","syscall");
	fprintf(f,"%s\n","jr $ra");
	fprintf(f,"%s\n","write:");
	fprintf(f,"%s\n","li $v0, 1");
	fprintf(f,"%s\n","syscall");
	fprintf(f,"%s\n","li $v0, 4");
	fprintf(f,"%s\n","la $a0, _ret");
	fprintf(f,"%s\n","syscall");
	fprintf(f,"%s\n","move $v0, $0");
	fprintf(f,"%s\n","jr $ra");
}
