%{
	#include "syntax.tab.h"
	#include "Tree.h"
	int yycolumn = 1;
	extern int isError;
	#define YY_USER_ACTION 	yylloc.first_line = yylloc.last_line = yylineno;\
	yylloc.first_column = yycolumn; \
	yylloc.last_column = yycolumn + yyleng -1; \
	yycolumn += yyleng;
	
	void makeNode(char *name,char* text,NodeKind kind);
%}

%option yylineno

delim	[ \t\n]
ws		{delim}+
letter	[a-zA-Z]
digit	[0-9]
id		({letter}|_)({letter}|{digit}|_)*
int		[0]|([1-9][0-9]*)
float	[+-]?{int}\.[0-9]*
floate	[+-]?([0-9]*\.[0-9]*)[Ee][+-]?[0-9]+
type	int|float
relop	>|<|>=|<=|==|!=
%%
\r			{/**/}
\n			{yycolumn=1;}
{ws}		{/*weisuo*/}
return		{makeNode("RETURN",yytext,T_OTHER);return RETURN;}
struct		{makeNode("STRUCT",yytext,T_OTHER);return STRUCT;}
if			{makeNode("IF",yytext,T_OTHER);return IF;}
else		{makeNode("ELSE",yytext,T_OTHER);return ELSE;}
while		{makeNode("WHILE",yytext,T_OTHER);return WHILE;}
{type}		{makeNode("TYPE",yytext,T_OTHER);return TYPE;}
{int}		{makeNode("INT",yytext,T_INT);return INT;}
{float}		{makeNode("FLOAT",yytext,T_FLOAT);return FLOAT;}
{floate}	{makeNode("FLOAT",yytext,T_FLOAT);return FLOAT;}
{id}		{makeNode("ID",yytext,T_ID);return ID;}
{relop}		{makeNode("RELOP",yytext,T_OTHER);return RELOP;}
";"			{makeNode("SEMI",yytext,T_OTHER);return SEMI;}
","			{makeNode("COMMA",yytext,T_OTHER);return COMMA;}
"="			{makeNode("ASSIGNOP",yytext,T_OTHER);return ASSIGNOP;}
"+"			{makeNode("PLUS",yytext,T_OTHER);return PLUS;}
"-"			{makeNode("MINUS",yytext,T_OTHER);return MINUS;}
"*"			{makeNode("STAR",yytext,T_OTHER);return STAR;}
"/"			{makeNode("DIV",yytext,T_OTHER);return DIV;}
"&&"		{makeNode("AND",yytext,T_OTHER);return AND;}
"||"		{makeNode("OR",yytext,T_OTHER);return OR;}
"."			{makeNode("DOT",yytext,T_OTHER);return DOT;}
"!"			{makeNode("NOT",yytext,T_OTHER);return NOT;}
"("			{makeNode("LP",yytext,T_OTHER);return LP;}
")"			{makeNode("RP",yytext,T_OTHER);return RP;}
"["			{makeNode("LB",yytext,T_OTHER);return LB;}
"]"			{makeNode("RB",yytext,T_OTHER);return RB;}
"{"			{makeNode("LC",yytext,T_OTHER);return LC;}
"}"			{makeNode("RC",yytext,T_OTHER);return RC;}
.			{isError=1;printf("Error type A at line %d:Mysterious character \'%s\'\n",yylineno,yytext);}

%%
void makeNode(char *name,char* text,NodeKind kind){
	yylval.typeNode=(Node*)malloc(sizeof(Node));
	yylval.typeNode->lineNum=yylineno;
	yylval.typeNode->type=kind;
	strcpy(yylval.typeNode->nodeName,name);
	strcpy(yylval.typeNode->text,text);
}
