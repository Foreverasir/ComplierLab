#include "symbols.h"
#include "semantics.h"
#include "assert.h"
#define MAX_DEPTH 64
extern int isError;
typedef struct Status{
	valueKind currentKind;
	Type_ *currentType;
	Val_ *currentValue;
	Func_ *currentFunc;
	Val_* currentParas[64];//Suppose it's enough

	int structFlag;
	int needFlag;
	int paraNum;
}Status;
Status sta;
Status s[MAX_DEPTH];
int currentDepth=0;
void staInit(){
	sta.currentKind=USERDEF;
	sta.currentType=NULL;sta.currentValue=NULL;sta.currentFunc=NULL;
	sta.structFlag=sta.needFlag=sta.paraNum=0;
	s[0]=sta;
}
void downStatus(){
	if(currentDepth==MAX_DEPTH){
		printf("currentDepth out of bound.\n");
		assert(0);
	}
	s[currentDepth]=sta;
	currentDepth++;
	sta.paraNum=0;
}
void upStatus(){
	if(currentDepth==0){
		printf("currentDepth reach -1.\n");
		assert(0);
	}
	currentDepth--;
	sta=s[currentDepth];
}
void setSNP(int a,int b,int c){sta.structFlag=a;sta.needFlag=b;sta.paraNum=c;}
void setSN(int a,int b){sta.structFlag=a;sta.needFlag=b;}
void setNP(int a,int b){sta.needFlag=a;sta.paraNum=b;}

/*check if the id exsit*/
Val_* expCheckVal(Node* head){
	Val_* v=getValue(head->text);
	if(v==NULL){
		printf("Error type 1 at Line %d: Undefined variable \"%s\".\n",head->lineNum,head->text);
		isError=1;
	}
	return v;
}
Func_* expCheckFunc(Node* head){
	Func_* f=getFunc(head->text);
	if(f==NULL){
		printf("Error type 2 at Line %d: Undefined function \"%s\".\n",head->lineNum,head->text);
		isError=1;
	}
	return f;
}
//=_=#
#define expInit() do{\
	*expValKind=USERDEF;\
	*expType=NULL;\
}while(0)

void semanticExp(Node* head,valueKind *expValKind,Type_ **expType);

void expSingle(Node* head,valueKind *expValKind,Type_ **expType){
	Val_* v;
	switch(head->child[0]->T){
		case ID_:
			v=(Val_*)expCheckVal(head->child[0]);
			if(v!=NULL){
				if(v->basicFlag==0){
					expInit();
					isError=1;
					printf("Error type 1 at Line %d: \"%s\" is a field of a struct.\n",head->lineNum,v->name);
				}else{
					*expValKind=v->kind;
					*expType=v->valType;
				}
			}else
				expInit();
			break;
		case INT_:*expValKind=INT;*expType=NULL;break;
		case FLOAT_:*expValKind=FLOAT;*expType=NULL;break;
	}
}
void expNotMinus(Node* head,valueKind *expValKind,Type_ **expType){
	valueKind vkind;
	Type_* t;
	semanticExp(head->child[1],&vkind,&t);
	if(head->child[0]->T==NOT_){
		if((vkind==USERDEF && t!=NULL) || vkind==FLOAT){
			expInit();
			isError=1;
			printf("Error type 7 at Line %d: \"!\"(not) only can be used before int.\n",head->lineNum);
		}else{
			*expValKind=vkind;*expType=t;
		}
	}else{
		if(vkind==USERDEF && t!=NULL){
			expInit();
			isError=1;
			printf("Error type 7 at Line %d: \"-\"(minus) only can be used before int or float.\n",head->lineNum);
		}else{
			*expValKind=vkind;*expType=t;
		}
	}
}
/*Whether obtain left*/
int judgeLeft(Node* head){
	switch(head->childNum){
		case 1:if(head->child[0]->T==ID_)return 1;break;//ID
		case 2:return 0;break;
		case 3:if(head->child[1]->T==DOT_)return 1;break;//Exp DOT ID
		case 4:if(head->child[1]->T==LB_)return 1;break;//Exp LB Exp RB
		default:printf("Oh god.\n");return 0;break;
	}
	return 0;
}

void expCompute(Node* head,valueKind *expValKind,Type_ **expType){
	valueKind k1,k2;
	Type_ *t1,*t2;
	switch(head->child[1]->T){
		case ASSIGNOP_:
			if(judgeLeft(head->child[0])==0){
				expInit();
				isError=1;
				printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",head->lineNum);
			}else{
				semanticExp(head->child[0],&k1,&t1);
				semanticExp(head->child[2],&k2,&t2);
				if((k1==USERDEF && t1==NULL) || (t2==NULL && k2==USERDEF))
					expInit();	
				else if(k1==k2 && typeEqual(t1,t2)){//Request 2.3
					*expValKind=k1;
					*expType=t1;
				}else{
					expInit();
					isError=1;
					printf("Error type 5 at Line %d: Type mismatched for assignment.\n",head->lineNum);
				}
			}break;
		case AND_:case OR_:
			semanticExp(head->child[0],&k1,&t1);
			semanticExp(head->child[2],&k2,&t2);
			if((k1==USERDEF && t1==NULL) || (t2==NULL && k2==USERDEF))
				expInit();
			else if(k1!=INT || k2!=INT){
				expInit();
				isError=1;
				printf("Error type 7 at Line %d: Only int can be used as a boolean result.\n",head->lineNum);
			}else{
				*expValKind=INT;
				*expType=NULL;
			}break;
		
		case Exp:semanticExp(head->child[1],expValKind,expType);break;
		case LP_:
			if(getValue(head->child[0]->text)!=NULL){
				expInit();
				isError=1;
				printf("Error type 11 at Line %d: \"%s\" is not a function but a variable.\n",head->lineNum,head->child[0]->text);
				return;
			}
			Func_* f=(Func_*)expCheckFunc(head->child[0]);
			if(f==NULL)expInit();
			else if(f->paraCount==0){
				*expValKind=f->returnKind;
				*expType=f->returnType;
			}else{
				expInit();
				isError=1;
				printf("Error type 9 at Line %d: Function \"%s\" is not applicable for the arguments.\n",head->lineNum,f->name);
			}
			break;
		printf("%d %d %d\n",head->child[1]->T,head->child[1]->childNum,head->child[1]->lineNum);
		case DOT_:
			semanticExp(head->child[0],&k1,&t1);
			if(k1!=USERDEF){
				expInit();
				isError=1;
				printf("Error type 13 at Line %d: Illegal use of \".\".\n",head->lineNum);
			}
			else if(t1==NULL)expInit();
			else if(t1->kind==ARRAY){
				if(t1->def.adl->kind==USERDEF && t1->def.adl->valType->kind==STRUCTURE){
					Val_* v=getValue(head->child[2]->text);
					int flag=0;
					for(int i=0;i<t1->def.adl->valType->def.sdl->defCount;i++){
						if(t1->def.adl->valType->def.sdl->defList[i]==v){
							*expValKind=v->kind;
							*expType=v->valType;
							flag=1;
						}
					}
					if(flag==0){
						expInit();
						isError=1;
						printf("Error type 14 at Line %d: Non-existent field \"%s\" for struct \"%s\".\n",head->lineNum,head->child[2]->text,t1->def.adl->valType->name);
					}
				}else{
					expInit();
					isError=1;
					printf("Error type 13 at Line %d: Illegal use of \".\".\n",head->lineNum);
				}
			}else{
				Val_* v=getValue(head->child[2]->text);
				int flag=0;
				for(int i=0;i<t1->def.sdl->defCount;i++){
					if(t1->def.sdl->defList[i]==v){
						*expValKind=v->kind;
						*expType=v->valType;
						flag=1;
					}
				}
				if(flag==0){
					expInit();
					isError=1;
					printf("Error type 14 at Line %d: Non-existent field \"%s\" for struct \"%s\".\n",head->lineNum,head->child[2]->text,t1->name);
				}
			}
			break;
		default://Include calculate and relop
			semanticExp(head->child[0],&k1,&t1);
			semanticExp(head->child[2],&k2,&t2);
			if((k1==USERDEF && t1==NULL) || (t2==NULL && k2==USERDEF)){
				expInit();
			}
			else if(k1==k2 && (k1==INT||k1==FLOAT)){
				*expValKind=k1;
				*expType=NULL;
			}else{
				expInit();
				isError=1;
				printf("Error type 7 at Line %d: Type mismatched for operands.Calculation needs two 'int' or two 'float'.\n",head->lineNum);
			}break;
	}
}
/* Exp->
 * Exp LB Exp RB
 * ID LP Args Rp
 * */
#define checkIfInt() do{\
	valueKind k;\
	Type_* t;\
	semanticExp(h->child[2],&k,&t);\
	if(k==USERDEF && t==NULL){expInit();return;}\
	else if(k!=INT){expInit();isError=1;printf("Error type 12 at Line %d: What inside the \"[ ]\" is not an integer.\n",head->lineNum);return;}\
}while(0)

void expLBExpRB(Node* head,valueKind *expValKind,Type_ **expType){
	Node* h=head;
	Node* parent[16];//Actually only 2
	int index=0;
	while(h->childNum==4 && h->child[0]->T==Exp){//Find array def
		parent[index]=h;
		index++;
		h=h->child[0];
	}
	if(h->childNum == 3 && h->child[1]->T==DOT_){//case struct.array 
		valueKind vkind;
		Type_* t;
		int flag=0;
		int flag2=0;
		semanticExp(h->child[0],&vkind,&t);
		Val_* v2=getValue(h->child[2]->text);
		if(t->kind==STRUCTURE){
			for(int i=0;i<t->def.sdl->defCount;i++){
				if(t->def.sdl->defList[i]==v2){
					flag2=1;
					if(v2->kind==USERDEF && v2->valType->kind==ARRAY){
						*expValKind=v2->valType->def.adl->kind;
						*expType=v2->valType->def.adl->valType;
						flag=1;	
					}
				}
			}
		}
		if(flag2==0){
			expInit();
			printf("Error type 14 at Line %d: Non-existent field \"%s\".\n",head->lineNum,h->child[2]->text);
			isError=1;
		}
		if(flag==0){
			expInit();
			printf("Error type 10 at Line %d: Wrong use \"[]\" after what is not an array.\n",head->lineNum);
			isError=1;
		}
		return;
	}
	else if(h->childNum!=1 || h->child[0]->T!=ID_){
		expInit();
		printf("Error type 10 at Line %d: Wrong use \"[]\" after what is not an array.\n",head->lineNum);
		isError=1;
		return;
	}
	Val_* v=(Val_*)expCheckVal(h->child[0]);
	if(v==NULL){expInit();return;}
	if(v->kind!=USERDEF || v->valType->kind!=ARRAY){
		expInit();
		isError=1;
		printf("Error type 10 at Line %d: Wrong use \"[]\" after what is not an array.\n",head->lineNum);
		return;
	}
	arrayDefList *current=v->valType->def.adl;
	if(index>0){
		index--;
		h=parent[index];
	}
	while(current->depth!=0 && h!=head){
		checkIfInt();
		if(index==0)assert(0);
		index--;
		h=parent[index];
		current=current->next;
	}
	if(current->depth==0 && h!=head){
		expInit();
		isError=1;
		printf("Error type 10 at Line %d: Wrong use \"[]\" after what is not an array(Maybe include too many '[]').\n",head->lineNum);
		return;
	}
	//the last[] has not been checked
	checkIfInt();
	
	if(h==head && current->depth==0){
		*expValKind=current->kind;
		*expType=current->valType;
		return;
	}
	else{
		Type_ *t=newType(NULL);
		addType(t);
		arrayDefList* copy=t->def.adl;
		current=current->next;
		while(current!=NULL){
			arrayDefList* copytemp=(arrayDefList*)malloc(sizeof(arrayDefList));
			memcpy(copytemp,current,sizeof(arrayDefList));
			copytemp->next=NULL;
			if(copy==NULL)t->def.adl=copytemp;
			else
				copy->next=copytemp;
			copy=copytemp;
			current=current->next;
		}
		*expValKind=USERDEF;
		*expType=t;
	}
}
void idLPArgsRP(Node* head,valueKind *expValKind,Type_ **expType){
	if(getValue(head->child[0]->text)!=NULL){
		isError=1;
		printf("Error type 11 at Line %d: \"%s\" is not a function\n",head->lineNum,head->child[0]->text);
		expInit();return;
	}
	Func_* f=(Func_*)expCheckFunc(head->child[0]);
	if(f==NULL){expInit();return;}
	//count args number
	int count=1;
	Node* args=head->child[2];
	while(args->childNum==3){
		count++;
		args=args->child[2];
	}
	if(count!=f->paraCount){
		isError=1;
		printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",head->lineNum,f->name);
		expInit();return;
	}
	//handle every argument
	args=head->child[2];
	for(int i=0;i<count;i++){
		valueKind k;
		Type_* t;
		semanticExp(args->child[0],&k,&t);
		if(k==USERDEF && t==NULL){expInit();return;}
		if(k!=f->kinds[i] || t!=f->para[i]){
			isError=1;
			printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",head->lineNum,f->name);
			expInit();return;
		}
		if(args->childNum==3)
			args=args->child[2];
	}
	*expValKind=f->returnKind;
	*expType=f->returnType;
}
void expFour(Node* head,valueKind *expValKind,Type_ **expType){
	if(head->child[0]->T==Exp)
		expLBExpRB(head,expValKind,expType);
	else
		idLPArgsRP(head,expValKind,expType);
}
void semanticExp(Node* head,valueKind *expValKind,Type_ **expType){
	switch(head->childNum){
		case 1:expSingle(head,expValKind,expType);break;
		case 2:expNotMinus(head,expValKind,expType);break;
		case 3:expCompute(head,expValKind,expType);break;
		case 4:expFour(head,expValKind,expType);break;
		default:printf("Unexpected Exp.\n");break;
	}	

}
void semanticSpecifier(Node *head){
	if(head->child[0]->T==TYPE_){
		sta.currentKind=(strcmp(head->child[0]->text,"int")==0)?INT:FLOAT;
		sta.currentType=NULL;
	}
	else{
		sta.currentKind=USERDEF;
		semanticMain(head->child[0]);
	}
}
void semanticExtDef(Node *head){
	if(head->child[1]->T==FunDec){
		semanticMain(head->child[0]);
		char *name=head->child[1]->child[0]->text;
		Func_ *f=getFunc(name);
		if(f!=NULL){
			printf("Error type 4 at Line %d: Redefined function \"%s\".\n",head->lineNum,name);
			isError=1;
		}
		else{
			f=newFunc(name);
			addFunc(f);
			sta.currentFunc=f;
			f->returnKind=sta.currentKind;
			f->returnType=sta.currentType;
			setNP(1,0);
			semanticMain(head->child[1]->child[2]);
			f->paraCount=sta.paraNum;
			f->kinds=(valueKind*)malloc(sta.paraNum*sizeof(valueKind));
			f->para=(Type_**)malloc(sta.paraNum*sizeof(Type_*));
			f->paraList=(Val_**)malloc(sta.paraNum*sizeof(Val_*));
			for(int i=0;i<sta.paraNum;i++){
				f->kinds[i]=sta.currentParas[i]->kind;
				f->para[i]=sta.currentParas[i]->valType;
				f->paraList[i]=sta.currentParas[i];
			}
			sta.needFlag=0;
			semanticMain(head->child[2]);
			sta.currentFunc=NULL;
		}
	}
	else{	
		for(int i=0;i<head->childNum;i++)
			semanticMain(head->child[i]);
	}
}
void semanticStructSpecifier(Node *head){
	if(head->childNum==2){
		char *name=head->child[1]->child[0]->text;
		sta.currentType=getType(name);
		if(sta.currentType==NULL){
			isError=1;
			printf("Error type 17 at Line %d: Undefined Structure \"%s\".\n",head->lineNum,name);
		}
	}
	else{
		char name[32];
		if(head->child[1]->T==None){
			getName(name);
		}
		else
			strcpy(name,head->child[1]->child[0]->text);
		sta.currentType=getType(name);
		if(sta.currentType!=NULL || getValue(name)!=NULL){
			isError=1;
			printf("Error type 16 at Line %d: Duplicated name \"%s\".\n",head->lineNum,name);
			sta.currentType=NULL;
		}
		else{ 
			sta.currentType=newType(name);
			addType(sta.currentType);
			downStatus();
			setSNP(1,1,0);
			semanticMain(head->child[3]);
			s[currentDepth-1].currentType->def.sdl->defCount=sta.paraNum;
			s[currentDepth-1].currentType->def.sdl->defList=(Val_**)malloc(sta.paraNum*sizeof(Val_*));
			for(int i=0;i<sta.paraNum;i++)
				s[currentDepth-1].currentType->def.sdl->defList[i]=sta.currentParas[i];
			setSN(0,0);
			upStatus();
		}
	}
}
void semanticVarDec(Node *head){
	Node *ID=head;
	for(;ID->childNum!=1;ID=ID->child[0]);
	ID=ID->child[0];
	Val_ *v=getValue(ID->text);
	if(v!=NULL){
		int i=-1;
		if(sta.structFlag==1)
			for(i=0;i<sta.paraNum;i++)
				if(strcmp(v->name,sta.currentParas[i]->name)==0)break;
		if(i>=0&&i<=sta.paraNum){
			isError=1;
			printf("Error type 15 at Line %d: Redefined field \"%s\".\n",head->lineNum,ID->text);
		}else{
			isError=1;
			printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",head->lineNum,ID->text);
		}
		sta.currentValue=NULL;
	}
	else if(!(sta.currentKind==USERDEF && sta.currentType==NULL)){//USERDEF but type is null: unexpected
		Val_ *v=newValue(ID->text);
		v->basicFlag=(sta.structFlag==0)?1:0;
		if(head->childNum!=1){//Array
			v->kind=USERDEF;
			v->valType=newType(NULL);
			addType(v->valType);
			Node *temp=head->child[0];
			arrayInit(v->valType,sta.currentKind,sta.currentType,atoi(head->child[2]->text));
			while(temp->childNum!=1){
				arrayGrow(v->valType,atoi(temp->child[2]->text));
				temp=temp->child[0];
			}
		}
		else{
			v->kind=sta.currentKind;
			v->valType=sta.currentType;
		}
		addValue(v);
		sta.currentValue=v;
		if(sta.needFlag>0){
			sta.currentParas[sta.paraNum]=v;
			sta.paraNum++;
		}
	}
}
void semanticStmt(Node *head){
	valueKind vkind;
	Type_ *t;
	switch(head->child[0]->T){
		case Exp:
			semanticExp(head->child[0],&vkind,&t);
			break;
		case CompSt:
			semanticMain(head->child[0]);break;
		case RETURN_:
			semanticExp(head->child[1],&vkind,&t);
			if(vkind!=USERDEF || t!=NULL){
				if(sta.currentFunc->returnKind!=vkind || sta.currentFunc->returnType!=t){
					isError=1;
					printf("Error type 8 at Line %d: Type mismatched or valid for return.\n",head->lineNum);
				}
			}
			break;
		default:
			semanticExp(head->child[2],&vkind,&t);
			//About Suppose2
			if(vkind!=INT && (vkind!=USERDEF || t!=NULL)){
				printf("Error type 7 at Line %d: Only int can be used as a boolean result\n",head->lineNum);
				isError=1;
			}
			for(int i=4;i<head->childNum;i++)
				semanticMain(head->child[i]);
			break;
	}
}
void semanticDec(Node *head){
	semanticMain(head->child[0]);
	if(head->childNum==3){
		if(sta.structFlag>0){
			printf("Error type 15 at Line %d: Cannot initialize a field when defining its structrue.\n",head->lineNum);
			isError=1;
		}
		else{
			valueKind vkind;
			Type_ *t;
			semanticExp(head->child[2],&vkind,&t);
			if(!(vkind==USERDEF && t==NULL) && sta.currentValue!=NULL){
				if(sta.currentValue->kind!=vkind || !typeEqual(sta.currentValue->valType,t)){
					printf("Error type 5 at Line %d: Type mismatched for assignment.\n",head->lineNum);
					isError=1;
				}
			}
		}
	}
}
void semanticMain(Node *head){
	if(head==NULL)return;
	switch(head->T){
		case CompSt:
			semanticMain(head->child[1]);
			semanticMain(head->child[2]);
			break;
		case Specifier:
			semanticSpecifier(head);break;
		case ExtDef:
			semanticExtDef(head);break;
		case StructSpecifier:
			semanticStructSpecifier(head);break;
		case VarDec:
			semanticVarDec(head);break;
		case Stmt:
			semanticStmt(head);break;
		case Dec:
			semanticDec(head);break;
		default:
			for(int i=0;i<head->childNum;i++)
				semanticMain(head->child[i]);
			break;
	}
}

