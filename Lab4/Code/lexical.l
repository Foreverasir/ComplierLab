%{
	#include "syntax.tab.h"
	#include "Tree.h"
	int yycolumn = 1;
	extern int isError;
	#define YY_USER_ACTION 	yylloc.first_line = yylloc.last_line = yylineno;\
	yylloc.first_column = yycolumn; \
	yylloc.last_column = yycolumn + yyleng -1; \
	yycolumn += yyleng;
	
	void makeNode(char *name,char* text,NodeKind kind,TYPES TP);
%}

%option yylineno

delim	[ \t\n]
ws		{delim}+
letter	[a-zA-Z]
digit	[0-9]
id		({letter}|_)({letter}|{digit}|_)*
int		[0]|([1-9][0-9]*)
float	[+-]?{int}\.[0-9]*
floate	[+-]?(([0-9]*\.[0-9]+)|([0-9]+\.[0-9]*))[Ee][+-]?[0-9]+
type	int|float
relop	>|<|>=|<=|==|!=
%%
\r			{/**/}
\n			{yycolumn=1;}
{ws}		{/*weisuo*/}
return		{makeNode("RETURN",yytext,T_OTHER,RETURN_);return RETURN;}
struct		{makeNode("STRUCT",yytext,T_OTHER,STRUCT_);return STRUCT;}
if			{makeNode("IF",yytext,T_OTHER,IF_);return IF;}
else		{makeNode("ELSE",yytext,T_OTHER,ELSE_);return ELSE;}
while		{makeNode("WHILE",yytext,T_OTHER,WHILE_);return WHILE;}
{type}		{makeNode("TYPE",yytext,T_OTHER,TYPE_);return TYPE;}
{int}		{makeNode("INT",yytext,T_INT,INT_);return INT;}
{float}		{makeNode("FLOAT",yytext,T_FLOAT,FLOAT_);return FLOAT;}
{floate}	{makeNode("FLOAT",yytext,T_FLOAT,FLOAT_);return FLOAT;}
{id}		{makeNode("ID",yytext,T_ID,ID_);return ID;}
{relop}		{makeNode("RELOP",yytext,T_OTHER,RELOP_);return RELOP;}
";"			{makeNode("SEMI",yytext,T_OTHER,SEMI_);return SEMI;}
","			{makeNode("COMMA",yytext,T_OTHER,COMMA_);return COMMA;}
"="			{makeNode("ASSIGNOP",yytext,T_OTHER,ASSIGNOP_);return ASSIGNOP;}
"+"			{makeNode("PLUS",yytext,T_OTHER,PLUS_);return PLUS;}
"-"			{makeNode("MINUS",yytext,T_OTHER,MINUS_);return MINUS;}
"*"			{makeNode("STAR",yytext,T_OTHER,STAR_);return STAR;}
"/"			{makeNode("DIV",yytext,T_OTHER,DIV_);return DIV;}
"&&"		{makeNode("AND",yytext,T_OTHER,AND_);return AND;}
"||"		{makeNode("OR",yytext,T_OTHER,OR_);return OR;}
"."			{makeNode("DOT",yytext,T_OTHER,DOT_);return DOT;}
"!"			{makeNode("NOT",yytext,T_OTHER,NOT_);return NOT;}
"("			{makeNode("LP",yytext,T_OTHER,LP_);return LP;}
")"			{makeNode("RP",yytext,T_OTHER,RP_);return RP;}
"["			{makeNode("LB",yytext,T_OTHER,LB_);return LB;}
"]"			{makeNode("RB",yytext,T_OTHER,RB_);return RB;}
"{"			{makeNode("LC",yytext,T_OTHER,LC_);return LC;}
"}"			{makeNode("RC",yytext,T_OTHER,RC_);return RC;}
.			{isError=1;printf("Error type A at line %d: Mysterious character \'%s\'\n",yylineno,yytext);}

%%
void makeNode(char *name,char* text,NodeKind kind,TYPES TP){
	yylval.typeNode=(Node*)malloc(sizeof(Node));
	yylval.typeNode->lineNum=yylineno;
	yylval.typeNode->type=kind;
	yylval.typeNode->T=TP;
	strcpy(yylval.typeNode->nodeName,name);
	strcpy(yylval.typeNode->text,text);
}
