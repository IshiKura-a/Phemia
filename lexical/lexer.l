%{
#define YYSTYPE char*
#define YYSTYPE double

#include <stdio.h>
#include "parser.hpp"

int yywrap();
void yyerror(char*);
%}

if                      if
else                    else
while                   while
for                     for
do                      do
break                   break
continue                continue
switch                  switch
case                    case
default                 default

int                     int
char                    char
double                  double
float                   float
boolean                 boolean
const                   const
void                    void
enum                    enum
string                  string

new                     new
class                   class
this                    this

try                     try
catch                   catch
throw                   throw

public                  public
private                 private
protected               protected

sizeof                  sizeof

true                    true
false                   false
num_integer             0 | -?[1-9][0-9]*
num_double              [0-9]+[.][0-9]+

newline                 \n | \r\n
ID                      [a-zA-Z_][0-9a-zA-Z]*

%%
"("                     {return LSB;}
")"                     {return RSB;}
"["                     {return LMB;}
"]"                     {return RMB;}
"{"                     {return LLB;}
"}"                     {return RLB;}

"."                     {return DOT;}
":"                     {return COLON;}
";"                     {return SEMI;}
","                     {return COMMA;}

"+"                     {return PLUS;}
"-"                     {return MINUS;}
"*"                     {return MUL;}
"/"                     {return DIV;}
"%"                     {return MOD;}
"^"                     {return XOR;}
"&&"                    {return AND;}
"||"                    {return OR;}

">"                     {return GT;}
">="                    {return GE;}
"<"                     {return LT;}
"<="                    {return LE;}
"!="                    {return NE;}
"=="                    {return EQ;}
"="                     {return ASSIGN;}
"\""                    {return QUOTE;}

"++"                    {return PPLUS;}
"--"                    {return SSUB;}




{if}                    {return IF;}
{else}                  {return ELSE;}
{while}                 {return WHILE;}
{for}                   {return FOR;}
{do}                    {return DO;}
{break}                 {return BREAK;}
{continue}              {return CONTINUE;}
{switch}                {return SWITCH;}
{case}                  {return CASE;}
{default}               {return DEFAULT;}

{int}                   {
                            yylval.type=(char*)malloc(strlen(yytext)+1);
                            strcpy(yylval.type,yytext);
                            return INT;
                        }
{char}                  {
                            yylval.type=(char*)malloc(strlen(yytext)+1);
                            strcpy(yylval.type,yytext);
                            return CHAR;
                        }
{double}                {
                            yylval.type=(char*)malloc(strlen(yytext)+1);
                            strcpy(yylval.type,yytext);
                            return DOUBLE;
                        }
{float}                 {
                            yylval.type=(char*)malloc(strlen(yytext)+1);
                            strcpy(yylval.type,yytext);
                            return FLOAT;
                        }
{boolean}               {
                            yylval.type=(char*)malloc(strlen(yytext)+1);
                            strcpy(yylval.type,yytext);
                            return BOOLEAN;
                        }
{const}                 {return CONST;}
{void}                  {return VOID;}
{enum}                  {return ENUM;}
{string}                {return STRING;}

{new}                   {return NEW;}
{class}                 {return CLASS;}
{this}                  {return THIS;}

{try}                   {return TRY;}
{catch}                 {return CATCH;}
{throw}                 {return THROW;}

{public}                {return PUBLIC;}
{private}               {return PRIVATE;}
{protected}             {return PROTECTED;}

{sizeof}                {return SIZEOF;}
{true}                  {yylval.number=1;return NUM;}
{false}                 {yylval.number=0;return NUM;}
{newline}               {}

{num_integer}           {yylval.number=atoi(yytext);return NUM;}
{num_double}            {yylval.number=atof(yytext);return NUM_DOUBLE;}

{ID}                    {return ID;}

.                       {
                            sprintf(msg,"yyerror:unexpected input %s",yytext);
                            yyerror(msg);
                        }

%%

int yywrap() {
    return 1;
}