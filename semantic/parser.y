%{
#include <iostream>
#include "node.h"

extern int yylex();
void yyerror(const char *s);

NBlock *programBlock;
extern int charPos;
extern int charLine;
extern std::string curToken;
%}

%union {
    std::string* val;
    std::string* type;
    Node *node;
    NBlock *block;
    NExpression *expr;
    NStatement *stmt;
    NIdentifier *id;
    NVariableDeclaration *varDecl;
    VariableList *varVec;
    ExpressionList *expVec;
    ArrayDimension *arrDim;
    int32_t token;
}

%token <token> LSB RSB LMB RMB LLB RLB DOT COLON SEMI
%token <token> PLUS MINUS MUL DIV MOD XOR AND OR QUOTE
%token <token> GT GE LT LE NE EQ ASSIGN NOT COMMA INC DEC

%token <token> IF ELSE WHILE FOR DO BREAK CONTINUE
%token <token> SWITCH CASE DEFAULT FUNCTION
%token <token> INT CHAR DOUBLE FLOAT BOOLEAN CONST
%token <token> VOID ENUM STRING NEW CLASS THIS
%token <token> TRY CATCH THROW PUBLIC PRIVATE PROTECTED
%token <token> SIZEOF RETURN
%token <val> ID

%token <val> INTEGER BOOL DNUMBER FNUMBER CHARACTER STR

%type <block> program blockedStmt stmts
%type <stmt> stmt funcDecl decl ifStmt forStmt nullableStmt
%type <stmt> whileStmt doWhileStmt
%type <id> type id basicType
%type <expr> exp expr term factor literal call assign
%type <varVec> declParamList
%type <expVec> paramList arrayIndices literalList literalArray
%type <varDecl> idDecl constIdDecl
%type <arrDim> arrayDimensions

%left PLUS MINUS MUL DIV MOD XOR AND OR NOT
%left GT GE LT LE NE EQ
%right ASSIGN

%%

program : stmts { programBlock = $1; }
    ;

stmts : stmts stmt { $1->statements.push_back($2); }
    | stmt { $$ = new NBlock(); $$->statements.push_back($1); }
    ;

stmt : decl { $$ = $1; }
    | ifStmt { $$ = $1; }
    | forStmt { $$ = $1; }
    | whileStmt { $$ = $1; }
    | doWhileStmt { $$ = $1; }
    | exp { $$ = new NExpressionStatement($1); }
    | BREAK { $$ = new NBreakStatement(); }
    | CONTINUE { $$ = new NContinueStatement(); }
    ;

whileStmt : WHILE LSB exp RSB blockedStmt { $$ = new NWhileStatement($3, $5); }
    ;

doWhileStmt : DO blockedStmt WHILE LSB exp RSB { $$ = new NDoWhileStatement($5, $2); }
    ;

nullableStmt : stmt { $$ = $1; }
    | { $$ = nullptr; }
    ;

ifStmt : IF LSB exp RSB blockedStmt { $$ = new NIfStatement($3, $5); }
    | IF LSB exp RSB blockedStmt ELSE blockedStmt { $$ = new NIfStatement($3, $5, $7); }
    | IF LSB exp RSB blockedStmt ELSE ifStmt { 
        auto elseBlock = new NBlock();
        elseBlock->statements.push_back($7);
        $$ = new NIfStatement($3, $5, elseBlock);
    }
    ;

forStmt : FOR LSB nullableStmt SEMI exp SEMI nullableStmt RSB blockedStmt { $$ = new NForStatement($3, $5, $7, $9); }
blockedStmt : LLB stmts RLB { $$ = $2; }
    | LLB RLB { $$ = new NBlock(); }
    ;

decl : idDecl { $$ = $1; }
    | constIdDecl { $$ = $1; }
    | funcDecl { $$ = $1; }
    | RETURN exp { $$ = new NReturnStatement(*$2); }
    ;

idDecl : type id { $$ = new NVariableDeclaration(false, *$1, *$2); }
    | type id ASSIGN exp { $$ = new NVariableDeclaration(false, *$1, *$2, $4); }
    ;

literalArray : LLB literalList RLB { $$ = $2; }
    ;
literalList : literalList COMMA literal { $$->push_back($3); }
    | literal { $$ = new ExpressionList(); $$->push_back($1); }
    ;

constIdDecl : CONST type id { $$ = new NVariableDeclaration(true, *$2, *$3); }
    | CONST type id ASSIGN exp { $$ = new NVariableDeclaration(true, *$2, *$3, $5); }
    ;

funcDecl : FUNCTION id LSB declParamList RSB COLON type blockedStmt {
    $$ = new NFunctionDeclaration(*$7, *$2, *$4, *$8); }
    ;

declParamList : idDecl { $$ = new VariableList(); $$->push_back($1); }
    | constIdDecl { $$ = new VariableList(); $$->push_back($1); }
    | declParamList COMMA constIdDecl { $1->push_back($3); }
    | declParamList COMMA idDecl { $1->push_back($3); }
    | { $$ = new VariableList(); }
    ;

exp : expr
    | exp GE expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
    | exp GT expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
    | exp LE expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
    | exp LT expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
    | exp NE expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
    | exp EQ expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
    ;
expr : expr PLUS term { $$ = new NBinaryOperator(*$1, $2, *$3); }
    | expr MINUS term { $$ = new NBinaryOperator(*$1, $2, *$3); }
    | expr OR term { $$ = new NBinaryOperator(*$1, $2, *$3); }
    | term { $$ = $1; }
    ;
term : term MUL factor { $$ = new NBinaryOperator(*$1, $2, *$3); }
    | term DIV factor { $$ = new NBinaryOperator(*$1, $2, *$3); }
    | term AND factor { $$ = new NBinaryOperator(*$1, $2, *$3); }
    | term MOD factor { $$ = new NBinaryOperator(*$1, $2, *$3); }
    | term XOR factor { $$ = new NBinaryOperator(*$1, $2, *$3); }
    | factor { $$ = $1; }
    ;
factor : literal { $$ = $1; }
    | id { $$ = $1; }
    | call { $$ = $1; }
    | assign { $$ = $1; }
    | LSB exp RSB { $$ = $2; }
    | NOT factor { $$ = new NUnaryOperator($1, *$2); }
    | MINUS factor { $$ = new NUnaryOperator($1, *$2); }
    | INC factor { $$ = new NIncOperator($1, *$2, true); }
    | DEC factor { $$ = new NDecOperator($1, *$2, true); }
    | factor INC { $$ = new NIncOperator($2, *$1, false); }
    | factor DEC { $$ = new NDecOperator($2, *$1, false); }
    | id DOT id {}
    | id DOT call {}
    | id arrayIndices { $$ = new NArrayElement(*$1, *$2); }
    | arrayDimensions type literalArray { $$ = new NArray($1, $2, $3); }
    | NEW arrayDimensions type LSB RSB { $$ = new NArray($2, $3); }
    | SIZEOF LSB type RSB {}
    ;
arrayDimensions : arrayDimensions LMB INTEGER RMB { $$->push_back($3); }
    | LMB INTEGER RMB { $$ = new ArrayDimension(); $$->push_back($2); }
    ;
arrayIndices : arrayIndices LMB exp RMB { $1->push_back($3); }
    | LMB exp RMB { $$ = new ExpressionList(); $$->push_back($2); }
    ;
call : id LSB RSB { $$ = new NFunctionCall(*$1, *(new ExpressionList())); }
    | id LSB paramList RSB { $$ = new NFunctionCall(*$1, *$3); }
    ;
assign : id ASSIGN exp { $$ = new NAssignment(*$1, *$3); }
    | id arrayIndices ASSIGN exp { $$ = new NArrayAssignment(*$1, *$2, *$4); }
    | id DOT id RMB ASSIGN exp {}
    ;
paramList : exp { $$ = new ExpressionList(); $$->push_back($1); }
    | paramList COMMA exp { $$->push_back($3); }
    ;
literal : INTEGER { $$ = new NInteger(*$1); }
    | BOOL { $$ = new NBoolean(*$1); }
    | STR { $$ = new NString(*$1); }
    | DNUMBER { $$ = new NDouble(*$1); }
    | FNUMBER { $$ = new NFloat(*$1); }
    | CHARACTER { $$ = new NChar(*$1); }
    ;

type : id { $$ = $1; }
    | basicType { $$ = $1; }
    | arrayDimensions basicType { $$ = new NArrayType($1, *$2); }
    ;

basicType : INT { $$ = new NIdentifier("int"); }
    | CHAR { $$ = new NIdentifier("char"); }
    | DOUBLE { $$ = new NIdentifier("double"); }
    | FLOAT { $$ = new NIdentifier("float"); }
    | BOOLEAN { $$ = new NIdentifier("boolean"); }
    | VOID { $$ = new NIdentifier("void"); }
    | STRING { $$ = new NIdentifier("string"); }
    ;

id : ID { $$ = new NIdentifier(*$1); };
%%

void yyerror(const char *s) {
    std::cout << "Token: " << curToken << std::endl
        << "Error: " << s << " at " << charLine << ":" << charPos << std::endl;
    std::exit(1);
}