%{
#include <stdio.h>
#include "parser.hpp"
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

bool                    true | false
integer                 0 | -?[1-9][0-9]*
newline                 \n | \r\n

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

{int}                   {return INT;}
{char}                  {return CHAR;}
{double}                {return DOUBLE;}
{float}                 {return FLOAT;}
{boolean}               {return BOOLEAN;}
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

[a-zA-Z_][0-9a-zA-Z]*   {return ID;}
%%

int yywrap() {
    return 1;
}