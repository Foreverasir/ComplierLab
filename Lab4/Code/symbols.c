#include "symbols.h"
Symbols symbols;
//help getName
int nameNum=0;

void initSymbols(){
	symbols.typeTable=NULL;
	symbols.valueTable=NULL;
	symbols.funcTable=NULL;
	Func_ *readFunc=newFunc("read");
	Func_ *writeFunc=newFunc("write");
	addFunc(readFunc);
	addFunc(writeFunc);
	readFunc->returnKind=INT;
	writeFunc->returnKind=INT;
	writeFunc->paraCount=1;
	writeFunc->kinds=(valueKind*)malloc(sizeof(valueKind));
	writeFunc->kinds[0]=INT;
	writeFunc->para=(Type_**)malloc(sizeof(Val_*));
	writeFunc->para[0]=NULL;
}
int typeEqualStruct(Type_ *t1,Type_ *t2){
	structDefList *s1=t1->def.sdl;
	structDefList *s2=t2->def.sdl;
	if(s1->defCount!=s2->defCount)return 0;
	int flag=0;
	for(int i=0;i<s1->defCount;i++){
		if(s1->defList[i]->kind!=USERDEF){
			if(s1->defList[i]->kind==s2->defList[i]->kind)flag=1;
			else return 0;
		}else if(s1->defList[i]->valType->kind==ARRAY){
			if(s2->defList[i]->kind==USERDEF && s2->defList[i]->valType->kind==ARRAY){
				arrayDefList *a1=s1->defList[i]->valType->def.adl;
				arrayDefList *a2=s2->defList[i]->valType->def.adl;
				if(a1->depth==a2->depth && a1->kind==a2->kind && a1->valType==a2->valType)flag=1;
				else return 0;
			}
			else
				return 0;
		}else{
			if(s2->defList[i]->kind==USERDEF && s2->defList[i]->valType->kind==STRUCTURE){
				int re=typeEqualStruct(s1->defList[i]->valType,s2->defList[i]->valType);
				if(re==1)flag=1;
				else return 0;
			}
			else 
				return 0;
		}
	}
	if(flag==1)return 1;
	else return 0;
}
int typeEqual(Type_ *t1,Type_ *t2){
	if(t1==t2)return 1;
	if(t1->kind!=t2->kind)return 0;
	if(t1->kind==STRUCTURE && t2->kind==STRUCTURE)return typeEqualStruct(t1,t2);	
	if(t1->def.adl->depth==t2->def.adl->depth && t1->def.adl->kind==t2->def.adl->kind && t2->def.adl->kind==USERDEF)return typeEqualStruct(t1,t2);
	else if(t1->def.adl->depth==t2->def.adl->depth && t1->def.adl->kind==t2->def.adl->kind && t1->def.adl->valType==t2->def.adl->valType)return 1;
	return 0;
}

void getName(char *name){
	strcpy(name,"_GZF_NAME_");
	int count=nameNum;
	int i=0;
	char a[5]={'\0'};
	if(count>1000)printf("Are you kidding me?\n");
	else{
		for(;i<4;i++){
			a[i]=count%10+'0';
			count/=10;
			if(count==0)break;
		}
	}
	nameNum++;
	strcat(name,a);
	strcat(name,"_");
}

void handleAorS(Type_ *p,int flag){
	p->kind=(flag==1)?STRUCTURE:ARRAY;
	if(flag==1){
		p->def.sdl=(structDefList*)malloc(sizeof(structDefList));
		p->def.sdl->defList=NULL;
		p->def.sdl->defCount=0;
	}else{
		p->def.adl=NULL;
		p->name[0]='\0';
	}
}
	
Type_* newType(const char* name){
	Type_ *p=(Type_*)malloc(sizeof(Type_));
	p->next=NULL;
	if(name!=NULL){
		strcpy(p->name,name);
		handleAorS(p,1);
	}else
		handleAorS(p,0);
	return p;
}
Val_* newValue(const char* name){
	Val_ *p=(Val_*)malloc(sizeof(Val_));
	strcpy(p->name,name);
	p->next=NULL;
	p->kind=INT;
	p->valType=NULL;
	p->basicFlag=1;
	return p;
}
Func_* newFunc(const char* name){
	Func_ *p=(Func_*)malloc(sizeof(Func_));
	strcpy(p->name,name);
	p->next=NULL;
	p->paraCount=0;
	p->kinds=NULL;
	p->returnType=NULL;
	p->para=NULL;
	p->paraList=NULL;
	return p;
}

Func_* getFunc(const char* name){
	Func_ *p=symbols.funcTable;
	while(p!=NULL){
		if(strcmp(name,p->name)==0)return p;
		p=p->next;
	}
	return NULL;
}
Type_* getType(const char* name){
	Type_ *p=symbols.typeTable;
	while(p!=NULL){
		if(strcmp(name,p->name)==0 && p->kind==STRUCTURE)return p;
		p=p->next;
	}
	return NULL;
}
Val_* getValue(const char* name){
	Val_ *p=symbols.valueTable;
	while(p!=NULL){
		if(strcmp(name,p->name)==0)return p;
		p=p->next;
	}
	Type_ *q=symbols.typeTable;
	while(q!=NULL){
		if(strcmp(name,q->name)==0 && q->kind==STRUCTURE){
			p=(Val_*)malloc(sizeof(Val_));
			p->basicFlag=2;
			return p;
		}
		q=q->next;
	}
	return NULL;
}

Type_* addType(Type_ *t){
	t->next=symbols.typeTable;
	symbols.typeTable=t;
	return t;
}
Val_* addValue(Val_ *v){
	v->next=symbols.valueTable;
	symbols.valueTable=v;
	return v;
}
Func_* addFunc(Func_ *f){
	f->next=symbols.funcTable;
	symbols.funcTable=f;
	return f;
}

int getStructSize(Type_ *t){
	if(t->kind==ARRAY)
		return (t->def.adl->number)*(t->def.adl->eleSize);
	structDefList *p=t->def.sdl;
	Val_* v;
	int count=0;
	for(int i=0;i<p->defCount;i++){
		v=p->defList[i];
		if(v->kind!=USERDEF)count+=4;
		else count+=getStructSize(v->valType);
	}
	return count;
}
int getOffset(Type_ *t,char *field0){
	structDefList* p=t->def.sdl;
	Val_* q;
	Val_* field=getValue(field0);
	int index=0;
	for(int i=0;i<p->defCount;i++){
		q=p->defList[i];
		if(q==field)break;
		if(q->kind!=USERDEF)
			index+=4;
		else
			index+=getStructSize(q->valType);
	}
	return index;
}
/*Array related functions*/
void arrayInit(Type_ *t,valueKind kind,Type_* valueType,int num){
	t->def.adl=(arrayDefList*)malloc(sizeof(arrayDefList));
	t->def.adl->depth=0;
	t->def.adl->number=num;
	t->def.adl->eleSize=(kind==USERDEF) ? getStructSize(valueType) : 4;
	t->def.adl->kind=kind;
	t->def.adl->valType=valueType;
	t->def.adl->next=NULL;
}
void arrayGrow(Type_ *t,int num){
	arrayDefList* p=(arrayDefList*)malloc(sizeof(arrayDefList));
	p->depth=t->def.adl->depth+1;
	p->number=num;
	p->eleSize=(t->def.adl->number)*(t->def.adl->eleSize);
	p->kind=t->def.adl->kind;
	p->valType=t->def.adl->valType;
	p->next=t->def.adl;
	t->def.adl=p;
}
