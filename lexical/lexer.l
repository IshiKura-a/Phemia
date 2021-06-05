%{
#include <iostream>
#include <string>
#include <vector>
#include "node.h"
#include "parser.hpp"

#define STRING_TOKEN    yylval.val = new std::string(yytext, yyleng)
#define TOKEN(t)        yylval.token = t

int charPos = 0;
int charLine = 1;
std::string curToken;
void incPos();
void incLine();
std::string* escapeStr(char ch[], int length);
%}

%option noyywrap

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
return                  return

int                     int
char                    char
double                  double
float                   float
boolean                 boolean
const                   const
void                    void
enum                    enum
string                  string
function                function

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

comment                 "//".*|[/][*][^*]*[*]+([^*/][^*]*[*]+)*[/]
bool                    true|false
integer                 0|[1-9][0-9]*
fNumber                 [0-9]+(\.[0-9]+)f
dNumber                 [0-9]+(\.[0-9]+)d?
character               '.'|'\\.'
str                     \".*\"

%%
{comment}               {incPos(); for(auto ch: std::string(yytext, yyleng)) if(ch == '\n') incLine(); }
{bool}                  {incPos(); STRING_TOKEN; return BOOL;}
{integer}               {incPos(); STRING_TOKEN; return INTEGER;}
{fNumber}               {incPos(); STRING_TOKEN; return FNUMBER;}
{dNumber}               {incPos(); STRING_TOKEN; return DNUMBER;}
{character}             {incPos(); yylval.val = escapeStr(yytext + 1, yyleng - 2); return CHARACTER;}
{str}                   {incPos(); yylval.val = escapeStr(yytext + 1, yyleng - 2); return STR;}

[ \t]                   {incPos();}
\n|\r\n                 {incLine();};
"++"                    {incPos(); TOKEN(INC); return INC;}
"--"                    {incPos(); TOKEN(DEC); return DEC;}
"("                     {incPos(); TOKEN(LSB); return LSB;}
")"                     {incPos(); TOKEN(RSB); return RSB;}
"["                     {incPos(); TOKEN(LMB); return LMB;}
"]"                     {incPos(); TOKEN(RMB); return RMB;}
"{"                     {incPos(); TOKEN(LLB); return LLB;}
"}"                     {incPos(); TOKEN(RLB); return RLB;}
"."                     {incPos(); TOKEN(DOT); return DOT;}
":"                     {incPos(); TOKEN(COLON); return COLON;}
";"                     {incPos(); TOKEN(SEMI); return SEMI;}
","                     {incPos(); TOKEN(COMMA); return COMMA;}
"+"                     {incPos(); TOKEN(PLUS); return PLUS;}
"-"                     {incPos(); TOKEN(MINUS); return MINUS;}
"*"                     {incPos(); TOKEN(MUL); return MUL;}
"/"                     {incPos(); TOKEN(DIV); return DIV;}
"%"                     {incPos(); TOKEN(MOD); return MOD;}
"^"                     {incPos(); TOKEN(XOR); return XOR;}
"&&"                    {incPos(); TOKEN(AND); return AND;}
"||"                    {incPos(); TOKEN(OR); return OR;}
">"                     {incPos(); TOKEN(GT); return GT;}
">="                    {incPos(); TOKEN(GE); return GE;}
"<"                     {incPos(); TOKEN(LT); return LT;}
"<="                    {incPos(); TOKEN(LE); return LE;}
"!="                    {incPos(); TOKEN(NE); return NE;}
"!"                     {incPos(); TOKEN(NOT); return NOT;}
"=="                    {incPos(); TOKEN(EQ); return EQ;}
"="                     {incPos(); TOKEN(ASSIGN); return ASSIGN;}

{if}                    {incPos(); TOKEN(IF); return IF;}
{else}                  {incPos(); TOKEN(ELSE); return ELSE;}
{while}                 {incPos(); TOKEN(WHILE); return WHILE;}
{for}                   {incPos(); TOKEN(FOR); return FOR;}
{do}                    {incPos(); TOKEN(DO); return DO;}
{break}                 {incPos(); TOKEN(BREAK); return BREAK;}
{continue}              {incPos(); TOKEN(CONTINUE); return CONTINUE;}
{switch}                {incPos(); TOKEN(SWITCH); return SWITCH;}
{case}                  {incPos(); TOKEN(CASE); return CASE;}
{default}               {incPos(); TOKEN(DEFAULT); return DEFAULT;}
{return}                {incPos(); TOKEN(RETURN); return RETURN;}

{int}                   {incPos(); TOKEN(INT); return INT;}
{char}                  {incPos(); TOKEN(CHAR); return CHAR;}
{double}                {incPos(); TOKEN(DOUBLE); return DOUBLE;}
{float}                 {incPos(); TOKEN(FLOAT); return FLOAT;}
{boolean}               {incPos(); TOKEN(BOOLEAN); return BOOLEAN;}
{const}                 {incPos(); TOKEN(CONST); return CONST;}
{void}                  {incPos(); TOKEN(VOID); return VOID;}
{enum}                  {incPos(); TOKEN(ENUM); return ENUM;}
{string}                {incPos(); TOKEN(STRING); return STRING;}
{function}              {incPos(); TOKEN(FUNCTION); return FUNCTION;}

{new}                   {incPos(); TOKEN(NEW); return NEW;}
{class}                 {incPos(); TOKEN(CLASS); return CLASS;}
{this}                  {incPos(); TOKEN(THIS); return THIS;}

{try}                   {incPos(); TOKEN(TRY); return TRY;}
{catch}                 {incPos(); TOKEN(CATCH); return CATCH;}
{throw}                 {incPos(); TOKEN(THROW); return THROW;}

{public}                {incPos(); TOKEN(PUBLIC); return PUBLIC;}
{private}               {incPos(); TOKEN(PRIVATE); return PRIVATE;}
{protected}             {incPos(); TOKEN(PROTECTED); return PROTECTED;}

{sizeof}                {incPos(); TOKEN(SIZEOF); return SIZEOF;}

[a-zA-Z_][0-9a-zA-Z]*   {incPos(); STRING_TOKEN; return ID;}

.                       {incPos(); printf("Unknown token!\n"); yyterminate();}
%%

void incPos() {
    charPos += yyleng;
    curToken = std::move(yytext);
}

void incLine() {
    charPos = 0;
    charLine++;
}

std::string *escapeStr(char ch[], int length)
{
    std::vector<char> cVec;
    int i;
    char prev = '\0';
    for (i = 0; i < length; i++) {
        if (prev != '\\') {
            if (ch[i] != '\\') {
                cVec.push_back(ch[i]);
            } else {
                prev = '\\';
            }
        } else {
            switch (ch[i])
            {
            case 'n': cVec.push_back('\n'); prev = '\0'; break;
            case 't': cVec.push_back('\t'); prev = '\0'; break;
            case 'b': cVec.push_back('\b'); prev = '\0'; break;
            case '\'': cVec.push_back('\''); prev = '\0'; break;
            case '\"': cVec.push_back('\"'); prev = '\0'; break;
            case '0': cVec.push_back('\0'); prev = '\0'; break;
            case '\\': cVec.push_back('\\'); prev = '\0'; break;
            default: prev = '\0'; break;
            }
        }
    }
    return new std::string(cVec.begin(), cVec.end());
}