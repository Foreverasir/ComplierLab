%{
	#include "Tree.h"
	#include "lex.yy.c"
	int isError=0;
	Node* root=NULL;
	char errorChar='\0';
%}

%union{ struct	TreeNode *typeNode;}
%token	<typeNode> INT
%token	<typeNode> FLOAT
%token	<typeNode> ID
%token	<typeNode> TYPE SEMI COMMA
%token	<typeNode> ASSIGNOP RELOP
%token	<typeNode> PLUS MINUS STAR DIV
%token	<typeNode> AND OR DOT NOT
%token	<typeNode> LP RP LB RB LC RC
%token	<typeNode> STRUCT RETURN IF ELSE WHILE

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT UMINUS
%left DOT LB RB LP RP

%type <typeNode> Program ExtDefList ExtDef ExtDecList
%type <typeNode> Specifier StructSpecifier OptTag Tag
%type <typeNode> VarDec FunDec VarList ParamDec
%type <typeNode> CompSt StmtList Stmt
%type <typeNode> DefList Def DecList Dec
%type <typeNode> Exp Args

%%
Program	: ExtDefList{
		$$=addNode("Program",Program,1,@$.first_line,$1);
		root=$$;
		}
ExtDefList	: ExtDef ExtDefList{
		$$=addNode("ExtDefList",ExtDefList,2,@$.first_line,$1,$2);}
			| {$$=addNode("ExtDefList",None,0,@$.first_line);}
			;
ExtDef	: Specifier ExtDecList SEMI{
		$$=addNode("ExtDef",ExtDef,3,@$.first_line,$1,$2,$3);}
		| Specifier SEMI{
		$$=addNode("ExtDef",ExtDef,2,@$.first_line,$1,$2);}
		| Specifier FunDec CompSt{
		$$=addNode("ExtDef",ExtDef,3,@$.first_line,$1,$2,$3);}
		| error SEMI{isError=1;/*errorChar=';';*/}
		;
ExtDecList	: VarDec{
		$$=addNode("ExtDecList",ExtDecList,1,@$.first_line,$1);}
			| VarDec COMMA ExtDecList{
		$$=addNode("ExtDecList",ExtDecList,3,@$.first_line,$1,$2,$3);}
			;
Specifier	: TYPE{
		$$=addNode("Specifier",Specifier,1,@$.first_line,$1);}
			| StructSpecifier{
		$$=addNode("Specifier",Specifier,1,@$.first_line,$1);}
			;
StructSpecifier : STRUCT OptTag LC DefList RC{
		$$=addNode("StructSepcifier",StructSpecifier,5,@$.first_line,$1,$2,$3,$4,$5);}
				| STRUCT Tag{
		$$=addNode("StructSepcifier",StructSpecifier,2,@$.first_line,$1,$2);}
				| STRUCT OptTag LC error RC{isError=1;errorChar='}';}
				;
OptTag		: ID{
		$$=addNode("OptTag",OptTag,1,@$.first_line,$1);}
			| {$$=addNode("OptTag",None,0,@$.first_line);}
			;
Tag : ID{
		$$=addNode("Tag",Tag,1,@$.first_line,$1);}
	;
VarDec	: ID{
		$$=addNode("VarDec",VarDec,1,@$.first_line,$1);}
		| VarDec LB INT RB{
		$$=addNode("VarDec",VarDec,4,@$.first_line,$1,$2,$3,$4);}
		| VarDec LB error RB{isError=1;/*errorChar=']';*/}
		;
FunDec	: ID LP VarList RP{
		$$=addNode("FunDec",FunDec,4,@$.first_line,$1,$2,$3,$4);}
		| ID LP RP{
		$$=addNode("FunDec",FunDec,3,@$.first_line,$1,$2,$3);}		
		| ID LP error RP{isError=1;/*errorChar=')';*/}
		| error RP{isError=1;}
		;
VarList	: ParamDec COMMA VarList{
		$$=addNode("VarList",VarList,3,@$.first_line,$1,$2,$3);}
		| ParamDec{
		$$=addNode("VarList",VarList,1,@$.first_line,$1);}
		;
ParamDec: Specifier VarDec{
		$$=addNode("ParamDec",ParamDec,2,@$.first_line,$1,$2);}
		;
CompSt	: LC DefList StmtList RC{
		$$=addNode("CompSt",CompSt,4,@$.first_line,$1,$2,$3,$4);}
		| error RC{isError=1;}
		;
StmtList: Stmt StmtList{
		$$=addNode("StmtList",StmtList,2,@$.first_line,$1,$2);}
		| {$$=addNode("StmtList",None,0,@$.first_line);}
		;
Stmt	: Exp SEMI{
		$$=addNode("Stmt",Stmt,2,@$.first_line,$1,$2);}
		| CompSt{
		$$=addNode("Stmt",Stmt,1,@$.first_line,$1);}
		| RETURN Exp SEMI{
		$$=addNode("Stmt",Stmt,3,@$.first_line,$1,$2,$3);}			
		| IF LP Exp RP Stmt	%prec LOWER_THAN_ELSE{
		$$=addNode("Stmt",Stmt,5,@$.first_line,$1,$2,$3,$4,$5);}
		| IF LP Exp RP Stmt ELSE Stmt{
		$$=addNode("Stmt",Stmt,7,@$.first_line,$1,$2,$3,$4,$5,$6,$7);}
		| WHILE LP Exp RP Stmt{
		$$=addNode("Stmt",Stmt,5,@$.first_line,$1,$2,$3,$4,$5);}	
		| error SEMI{isError=1;/*errorChar=';';*/}
		;
DefList	: Def DefList{
		$$=addNode("DefList",DefList,2,@$.first_line,$1,$2);}
		|{$$=addNode("DefList",None,0,@$.first_line);}
		;
Def	: Specifier DecList SEMI{
		$$=addNode("Def",Def,3,@$.first_line,$1,$2,$3);}
	;
DecList	: Dec{
		$$=addNode("DecList",DecList,1,@$.first_line,$1);}
		| Dec COMMA DecList{
		$$=addNode("DecList",DecList,3,@$.first_line,$1,$2,$3);}
		;
Dec	: VarDec{
	$$=addNode("Dec",Dec,1,@$.first_line,$1);}
	| VarDec ASSIGNOP Exp{
	$$=addNode("Dec",Dec,3,@$.first_line,$1,$2,$3);}
	| VarDec error Exp{isError=1;}
	;
Exp	: Exp ASSIGNOP Exp{
	$$=addNode("Exp",Exp,3,@$.first_line,$1,$2,$3);}
	| Exp AND Exp{
	$$=addNode("Exp",Exp,3,@$.first_line,$1,$2,$3);}
	| Exp OR Exp{
	$$=addNode("Exp",Exp,3,@$.first_line,$1,$2,$3);}
	| Exp RELOP Exp{
	$$=addNode("Exp",Exp,3,@$.first_line,$1,$2,$3);}
	| Exp PLUS Exp{
	$$=addNode("Exp",Exp,3,@$.first_line,$1,$2,$3);}
	| Exp MINUS Exp{
	$$=addNode("Exp",Exp,3,@$.first_line,$1,$2,$3);}
	| Exp STAR Exp{
	$$=addNode("Exp",Exp,3,@$.first_line,$1,$2,$3);}
	| Exp DIV Exp{
	$$=addNode("Exp",Exp,3,@$.first_line,$1,$2,$3);}
	| LP Exp RP{
	$$=addNode("Exp",Exp,3,@$.first_line,$1,$2,$3);}
	| MINUS Exp{
	$$=addNode("Exp",Exp,2,@$.first_line,$1,$2);}
	| NOT Exp{
	$$=addNode("Exp",Exp,2,@$.first_line,$1,$2);}
	| ID LP Args RP{
	$$=addNode("Exp",Exp,4,@$.first_line,$1,$2,$3,$4);}
	| ID LP RP{
	$$=addNode("Exp",Exp,3,@$.first_line,$1,$2,$3);}
	| Exp LB Exp RB{
	$$=addNode("Exp",Exp,4,@$.first_line,$1,$2,$3,$4);}
	| Exp DOT ID{
	$$=addNode("Exp",Exp,3,@$.first_line,$1,$2,$3);}
	| ID{
	$$=addNode("Exp",Exp,1,@$.first_line,$1);}
	| INT{
	$$=addNode("Exp",Exp,1,@$.first_line,$1);}
	| FLOAT{
	$$=addNode("Exp",Exp,1,@$.first_line,$1);}
	| LP error RP{isError=1;/*errorChar=')';*/}
	| Exp LB error RB{isError=1;/*errorChar=']';*/}
	| error DOT ID{isError=1;}
	| error AND Exp{isError=1;}
	| error DIV Exp{isError=1;}
	| error STAR Exp{isError=1;}
	| error RELOP Exp{isError=1;}
	| error PLUS Exp{isError=1;}
	| error MINUS Exp{isError=1;}
	| error ASSIGNOP Exp{isError=1;}
	| error OR Exp{isError=1;}
	;
Args: Exp COMMA Args{
	$$=addNode("Args",Args,3,@$.first_line,$1,$2,$3);}
	| Exp{
	$$=addNode("Args",Args,1,@$.first_line,$1);}
	;
%%

yyerror(char *msg){
	if(errorChar!='\0'){
		fprintf(stderr,"Error type B at line %d: Missing \"%c\".\n",yylineno,errorChar);
		errorChar='\0';
	}
	else		
		fprintf(stderr,"Error type B at line %d: %s\n",yylineno,msg);
}


