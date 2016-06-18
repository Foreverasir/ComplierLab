#include "Tree.h"
#include "irbuffer.h"
#include "symbols.h"
#include <stdio.h>

static Func_* currentFunc=NULL;
static Type_* currentStruct=NULL;//request 3.1
static int currentArrayEleSize=4;

static Type_* staStruct[16];
static int staSize[16];
static int staIndex=0;
static void saveSta(){
	if(staIndex>15){
		printf("To much status.\n");
		exit(0);
	}
	staSize[staIndex]=currentArrayEleSize;
	staStruct[staIndex]=currentStruct;
	staIndex++;
}
static void loadSta(){
	staIndex--;
	currentArrayEleSize=staSize[staIndex];
	currentStruct=staStruct[staIndex];
}

void translateExp(Node* head,char* place,int flag);
void translateArgs(Node* head,Val_** args,int count);
void translateCond(Node* head,char* labelTrue,char* labelFalse);

/*get args addr*/
void getAddr(Node* head,char* place){
	if(head->childNum==1){
		Val_ *v=getValue(head->child[0]->text);
		if(v->valType->kind==STRUCTURE)
			currentStruct=v->valType;
		else currentArrayEleSize=v->valType->def.adl->eleSize;
		for(int i=0;i<currentFunc->paraCount;i++){
			if(currentFunc->paraList[i]==v){
				strcpy(place,head->child[0]->text);
				return;
			}
		}
		sprintf(place,"&%s",head->child[0]->text);
	}
	else if(head->childNum==4 && head->child[1]->T==LB_){//Exp LB Exp RB
		char t1[32],t2[32];
		getAddr(head->child[0],t1);
		saveSta();
		translateExp(head->child[2],t2,0);
		loadSta();
		if(t2[0]=='#' && t2[1]=='0'){
			strcpy(place,t1);
		}
		else if(t2[0]=='#'){
			int i=strtol(&t2[1],NULL,10);
			char t[32],tindex[32];
			newTemp(t);
			sprintf(tindex,"#%d",i * currentArrayEleSize);
			addCode(5,t,":=",t1,"+",tindex);
			strcpy(place,t);
		}
		else{
			char t[32],tindex[32],number[32];
			sprintf(number,"#%d",currentArrayEleSize);
			newTemp(t);
			newTemp(tindex);
			addCode(5,tindex,":=",t2,"*",number);
			addCode(5,t,":=",t1,"+",tindex);
			strcpy(place,t);
		}
	}
	else{//Struct id.field
		char base[32],offset[32];
		getAddr(head->child[0],base);
		int offsetIndex=getOffset(currentStruct,head->child[2]->text);
		sprintf(offset,"#%d",offsetIndex);
		newTemp(place);
		addCode(5,place,":=",base,"+",offset);
		Val_ *v=getValue(head->child[2]->text);//get field
		if(v->kind==USERDEF){
			if(v->valType->kind==STRUCTURE)
				currentStruct=v->valType;
			else{
				currentArrayEleSize=v->valType->def.adl->eleSize;
				if(v->valType->def.adl->kind==USERDEF)
					currentStruct=v->valType->def.adl->valType;
			}
		}
	}
}

void translateExpLpRp(Node* head,char* place,int flag){
	char t[32];
	if(head->childNum==3){
		if(place==NULL){
			//printf("place is NULL\n");
			if(strcmp(head->child[0]->text,"read")==0)return;
			newTemp(t);
			addCode(4,t,":=","CALL",head->child[0]->text);
		}else{
			if(flag==0)newTemp(place);
			if(strcmp(head->child[0]->text,"read")==0)
				addCode(2,"READ",place);
			else
				addCode(4,place,":=","CALL",head->child[0]->text);
		}
	}
	else{
		if(strcmp(head->child[0]->text,"write")==0){
			translateExp(head->child[2]->child[0],t,0);
			addCode(2,"WRITE",t);
			if(place!=NULL){
				if(flag=1)
					addCode(3,place,":=","#0");
				else{
					place[0]='#';place[1]='0';place[2]='\0';
				}
			}
		}
		else{
			Func_ *f=getFunc(head->child[0]->text);
			translateArgs(head->child[2],f->paraList,0);
			if(place==NULL){
				newTemp(t);
				addCode(4,t,":=","CALL",head->child[0]->text);
			}
			else{
				if(flag==0)newTemp(place);
				addCode(4,place,":=","CALL",head->child[0]->text);
			}
		}
	}
}

void translateExpAssignop(Node* head,char* place,int flag){
	char t[32];
	translateExp(head->child[0],t,0);
	translateExp(head->child[2],t,1);//left is given
	if(place!=NULL){
		if(flag==0){
			strcpy(place,t);
		}	
		else addCode(3,place,":=",t);
	}
}

void translateExpArray(Node* head,char* place,int flag){
	char t1[32],t2[32],t[32];
	getAddr(head->child[0],t1);
	translateExp(head->child[2],t2,0);
	if(t2[0]=='#'){
		if(t2[1]=='0'){
			if(t1[0]=='&')
				strcpy(t,&t1[1]);
			else sprintf(t,"*%s",t1);
			if(flag==0){ 
				strcpy(place,t);
			}
			else addCode(3,place,":=",t);
		}else{
			int i=strtol(&t2[1],NULL,10);
			char tindex[32];
			sprintf(tindex,"#%d",i*currentArrayEleSize);
			newTemp(t);
			addCode(5,t,":=",t1,"+",tindex);
			char result[32];
			sprintf(result,"*%s",t);
			if(flag==0){
				strcpy(place,result);
			}
			else addCode(3,place,":=",result);
		}
	}
	else{
		char tindex[32],result[32],number[32];
		sprintf(number,"#%d",currentArrayEleSize);
		newTemp(t);newTemp(tindex);
		addCode(5,tindex,":=",t2,"*",number);
		addCode(5,t,":=",t1,"+",tindex);
		sprintf(result,"*%s",t);
		if(flag==0) strcpy(place,result);
		else addCode(3,place,":=",result);
	}
}
void translateExpBool(Node* head,char* place,int flag){
	char label1[32],label2[32];
	newLabel(label1);
	newLabel(label2);
	if(flag==0)newTemp(place);
	addCode(3,place,":=","#0");
	translateCond(head,label1,label2);
	addCode(3,"LABEL",label1,":");
	addCode(3,place,":=","#1");
	addCode(3,"LABEL",label2,":");
}
int checkZero(Node* head,char* place,int flag,char t[]){
	if((t[0]=='#' && t[1]=='0') && (head->child[1]->T==STAR_ || head->child[1]->T==DIV_)){
		if(flag==0){
			place[0]='#';
			place[1]='0';
			place[2]='\0';
		}
		else
			addCode(3,place,":=","#0");
		return 1;
	}
	return 0;
}
void translateExpCal(Node* head,char* place,int flag){
	char t1[32],t2[32];
	int cflag=0;
	translateExp(head->child[0],t1,0);
	cflag=checkZero(head,place,flag,t1);
	translateExp(head->child[2],t2,0);
	cflag=checkZero(head,place,flag,t2);
	if(cflag==1)return;
	if(t1[0]=='#' && t2[0]=='#'){
		int i1=strtol(&t1[1],NULL,10),i2=strtol(&t2[1],NULL,10);
		char t[32];
		switch(head->child[1]->T){
			case PLUS_:
				if(flag==0) sprintf(place,"#%d",i1+i2);
				else{
					sprintf(t,"#%d",i1+i2);
					addCode(3,place,":=",t);
				}
				break;
			case MINUS_:
				if(flag==0) sprintf(place,"#%d",i1-i2);
				else{
					sprintf(t,"#%d",i1-i2);
					addCode(3,place,":=",t);
				}
				break;
			case STAR_:
				if(flag==0) sprintf(place,"#%d",i1*i2);
				else{
					sprintf(t,"#%d",i1*i2);
					addCode(3,place,":=",t);
				}
				break;
			case DIV_:
				if(flag==0) sprintf(place,"#%d",i1/i2);
				else{
					sprintf(t,"#%d",i1/i2);
					addCode(3,place,":=",t);
				}
				break;
			default:return;break;
		}
	}
	else if(t2[0]=='#' && t2[1]=='0'){
		if(flag==0) strcpy(place,t1);
		else addCode(3,place,":=",t1);
	}
	else if(t1[0]=='#' && t1[1]=='0' && head->child[1]->T==PLUS_){
		if(flag==0) strcpy(place,t2);
		else addCode(3,place,":=",t2);
	}
	else{
		char opra[2];
		switch(head->child[1]->T){
			case PLUS_:opra[0]='+';break;
			case MINUS_:opra[0]='-';break;
			case STAR_:opra[0]='*';break;
			case DIV_:opra[0]='/';break;
			default:return;break;
		}
		opra[1]='\0';
		if(flag==0)
			newTemp(place);
		addCode(5,place,":=",t1,opra,t2);
	}
}

/*About flag
 *flag=0 place is just a address
 *flag=1 place is given
 */
void translateExp(Node* head,char* place,int flag){
	//printf("0 %s %s %d\n",head->nodeName,head->text,head->childNum);
	if(head->child[0]->T==LP_){
		//printf("here\n");
		return translateExp(head->child[1],place,flag);
	}else if(head->childNum==3 && head->child[1]->T==ASSIGNOP_){
		//printf("here1\n");
		translateExpAssignop(head,place,flag);
	}else if(head->childNum>=3 && head->child[1]->T==LP_){
		//printf("here2\n");
		translateExpLpRp(head,place,flag);
	}
	if(place==NULL)return;
	char t[32];
	if(head->child[0]->T==FLOAT_){
		printf("Illegal input Float at line %d.\n",head->child[0]->lineNum);
		exit(0);
	}
	if(head->child[0]->T==INT_){
		if(flag==0)
			sprintf(place,"#%d",atoi(head->child[0]->text));
		else{
			sprintf(t,"#%d",atoi(head->child[0]->text));
			addCode(3,place,":=",t);
		}
	}
	else if(head->childNum==1 && head->child[0]->T==ID_){
		if(flag==0){
			strcpy(place,head->child[0]->text);
		}
		else
			addCode(3,place,":=",head->child[0]->text);
	}
	else if(head->child[0]->T==MINUS_){
		translateExp(head->child[1],t,0);
		if(t[0]=='#'){
			int index=strtol(&t[1],NULL,10);
			if(flag==0)	sprintf(place,"#%d",-index);
			else{
				sprintf(t,"#%d",-index);
				addCode(3,place,":=",t);
			}
		}else{
			if(flag==0) newTemp(place);
			addCode(5,place,":=","#0","-",t);
		}
	}
	else if(head->childNum==3 && head->child[1]->T==DOT_){
		char left[32],result[32],offset[32];
		getAddr(head->child[0],left);
		int offsetIndex=getOffset(currentStruct,head->child[2]->text);
		sprintf(offset,"#%d",offsetIndex);
		newTemp(t);
		addCode(5,t,":=",left,"+",offset);
		sprintf(result,"*%s",t);
		if(flag==0){ 
			strcpy(place,result);
		}
		else addCode(3,place,":=",result);
	}
	else if(head->childNum==4 && head->child[1]->T==LB_)
		translateExpArray(head,place,flag);
	else if(head->child[0]->T==NOT_ || head->child[1]->T==RELOP_ || head->child[1]->T==OR_ || head->child[1]->T==AND_)
		translateExpBool(head,place,flag);
	else if(head->childNum==3 && (head->child[1]->T==PLUS_ || head->child[1]->T==MINUS_ || head->child[1]->T==STAR_ || head->child[1]->T==DIV_))
		translateExpCal(head,place,flag);
}

/*get the current aegs table and the current index*/
void translateArgs(Node* head,Val_** args,int count){
	if(head->childNum==3)
		translateArgs(head->child[2],args,count+1);
	char t[32];
	if(args[count]->kind!=USERDEF){
		translateExp(head->child[0],t,0);
		addCode(2,"ARG",t);
	}
	else if(args[count]->valType->kind==STRUCTURE){
		getAddr(head->child[0],t);
		addCode(2,"ARG",t);
	}
	else{
		printf("Cannot translate: Args contains array.\n");
		exit(0);
	}
} 

/*Labels have been provided*/
void translateCond(Node* head,char* labelTrue,char* labelFalse){
	if(head->child[0]->T==NOT_)
		return translateCond(head->child[1],labelFalse,labelTrue);
	char t1[32],t2[32];
	char label[32];
	if(head->childNum==3 && head->child[1]->T==RELOP_){
		translateExp(head->child[0],t1,0);
		translateExp(head->child[2],t2,0);
		if(t1[0]=='#' && t2[0]=='#'){
			int i1=strtol(&t1[1],NULL,10);
			int i2=strtol(&t2[1],NULL,10);
			if(strcmp(head->child[1]->text,">")==0)
				addCode(2,"GOTO",(i1>i2) ? labelTrue : labelFalse);
			if(strcmp(head->child[1]->text,"<")==0)
				addCode(2,"GOTO",(i1<i2) ? labelTrue : labelFalse);
			if(strcmp(head->child[1]->text,">=")==0)
				addCode(2,"GOTO",(i1>=i2) ? labelTrue : labelFalse);
			if(strcmp(head->child[1]->text,"<=")==0)
				addCode(2,"GOTO",(i1<=i2) ? labelTrue : labelFalse);
			if(strcmp(head->child[1]->text,"==")==0)		
				addCode(2,"GOTO",(i1==i2) ? labelTrue : labelFalse);
			if(strcmp(head->child[1]->text,"!=")==0)
				addCode(2,"GOTO",(i1!=i2) ? labelTrue : labelFalse);
		}else{
			addCode(6,"IF",t1,head->child[1]->text,t2,"GOTO",labelTrue);
			addCode(2,"GOTO",labelFalse);
		}
	}
	else if(head->childNum==3 && head->child[1]->T==AND_){
		newLabel(label);
		translateCond(head->child[0],label,labelFalse);
		addCode(3,"LABEL",label,":");
		translateCond(head->child[2],labelTrue,labelFalse);
	}
	else if(head->childNum==3 && head->child[1]->T==OR_){
		newLabel(label);
		translateCond(head->child[0],labelTrue,label);
		addCode(3,"LABEL",label,":");
		translateCond(head->child[2],labelTrue,labelFalse);
	}
	else{
		translateExp(head,t1,0);
		if(t1[0]=='#'){
			if(t1[1]!='0')addCode(2,"GOTO",labelTrue);
			else addCode(2,"GOTO",labelFalse);
		}else{
			addCode(6,"IF",t1,"!=","#0","GOTO",labelTrue);
			addCode(2,"GOTO",labelFalse);
		}
	}
}

void translateMain(Node *head){
	if(head==NULL){
		printf("Null head\n");
		return;
	}
	Func_ *func;
	Val_ *v;
	Node *p;
	switch(head->T){
		case Specifier:
			return;break;
		case FunDec:
			func=getFunc(head->child[0]->text);
			addCode(3,"FUNCTION",head->child[0]->text,":");
			for(int i=0;i<func->paraCount;i++)
				addCode(2,"PARAM",func->paraList[i]->name);
			currentFunc=func;
			break;
		case Dec:
			p=head->child[0]->child[0];
			if(p->T!=ID_){
				p=p->child[0];
				if(p->T!=ID_){
					printf("Cannot translate: Code contains variables of multi-dimension array type or array-parameters.\n");
					exit(0);
				}
			}
			v=getValue(p->text);
			if(v->kind==USERDEF){
				char t[16];
				itoa(getStructSize(v->valType),t,10);
				addCode(3,"DEC",v->name,t);
			}
			if(head->childNum==3)
				translateExp(head->child[2],v->name,1);
			break;
		case Stmt:
			switch(head->child[0]->T){
				case Exp:translateExp(head->child[0],NULL,0);break;
				case CompSt:translateMain(head->child[0]);break;
				case RETURN_:{
								 char t[32];
								 translateExp(head->child[1],t,0);
								 addCode(2,"RETURN",t);
							 }break;
				case IF_:{
							char label1[32],label2[32],label3[32];
							newLabel(label1);newLabel(label2);
							if(head->childNum==7)
								newLabel(label3);
							translateCond(head->child[2],label1,label2);
							addCode(3,"LABEL",label1,":");
							translateMain(head->child[4]);
							if(head->childNum==7)
								addCode(2,"GOTO",label3);
							addCode(3,"LABEL",label2,":");
							if(head->childNum==7){
								translateMain(head->child[6]);
								addCode(3,"LABEL",label3,":");
							}
						 }break;
				case WHILE_:{
								char label1[32],label2[32],label3[32];
								newLabel(label1);newLabel(label2);newLabel(label3);
								addCode(3,"LABEL",label1,":");
								translateCond(head->child[2],label2,label3);
								addCode(3,"LABEL",label2,":");
								translateMain(head->child[4]);
								addCode(2,"GOTO",label1);
								addCode(3,"LABEL",label3,":");
							}break;
				default:break;
			}
			break;
		default:
			for(int i=0;i<head->childNum;i++)
				translateMain(head->child[i]);
			break;
	}
}
