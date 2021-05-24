%{
#include <stdio.h>

int yylex(void);
void yyerror(char* s);
%}

%token  LSB RSB LMB RMB LLB RLB DOT COLON SEMI COMMA PLUS
        MINUS MUL DIV MOD XOR AND OR GT GE LT LE NE EQ ASSIGN
        QUOTE IF ELSE WHILE FOR DO BREAK CONTINUE SWITCH
        CASE DEFAULT INT CHAR DOUBLE FLOAT BOOLEAN CONST
        ENUM STRING NEW CLASS THIS TRY CATCH THROW PUBLIC
        PRIVATE PROTECTED SIZEOF VOID ID

%%
start: test
    | test start;
test: LSB {printf("LSB ");}|RSB{printf("RSB ");}|LMB{printf("LMB ");}|
        RMB{printf("RMB ");}|LLB{printf("LLB ");}|RLB{printf("RLB ");}|
        DOT{printf("DOT ");}|COLON{printf("COLON ");}|SEMI{printf("SEMI ");}|
        COMMA{printf("COMMA ");}|PLUS{printf("PLUS ");}|MINUS{printf("MINUS ");}|
        MUL{printf("MUL ");}|DIV{printf("DIV ");}|MOD{printf("MOD ");}|
        XOR{printf("XOR ");}|AND{printf("AND ");}|OR{printf("OR ");}|GT{printf("GT ");}|
        GE{printf("GE ");}|LT{printf("LT ");}|LE{printf("LE ");}|NE{printf("NE ");}|
        EQ{printf("EQ ");}|ASSIGN{printf("ASSIGN ");}|QUOTE{printf("QUOTE ");}|
        IF{printf("IF ");}|ELSE{printf("ELSE ");}|WHILE{printf("WHILE ");}|
        FOR{printf("FOR ");}|DO{printf("DO ");}|BREAK{printf("BREAK ");}|
        CONTINUE{printf("CONTINUE ");}|SWITCH{printf("SWITCH ");}|CASE{printf("CASE ");}|
        DEFAULT{printf("DEFAULT ");}|INT{printf("INT ");}|CHAR{printf("CHAR ");}|
        DOUBLE{printf("DOUBLE ");}|FLOAT{printf("FLOAT ");}|BOOLEAN{printf("BOOLEAN ");}|
        CONST{printf("CONST ");}|ENUM{printf("ENUM ");}|STRING{printf("STRING ");}|
        NEW{printf("NEW ");}|CLASS{printf("CLASS ");}|THIS{printf("THIS ");}|TRY{printf("TRY ");}|
        CATCH{printf("CATCH ");}|THROW{printf("THROW ");}|PUBLIC{printf("PUBLIC ");}|
        PRIVATE{printf("PRIVATE ");}|PROTECTED{printf("PROTECTED ");}|SIZEOF{printf("SIZEOF ");}|
        VOID{printf("VOID ");}|ID{printf("ID ");};
%%

int main() {
    yyparse();
}

void yyerror(char* s) {
    fprintf(stderr, "%s\n", s);
}