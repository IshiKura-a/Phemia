```enbf
program -> {stmt}

stmt -> decl SEMI | def | exp SEMI | ifStmt | forStmt
    | whileStmt | switchStmt | BREAK SEMI | CONTINUE SEMI
    | retStmt SEMI | IOStmt SEMI | throwStmt SEMI
    | tryStmt | assign SEMI
blockedStmt -> LLB {oneStmt} RLB


decl -> idDecl | constIdDecl | funcDecl | enumDecl
idDecl -> type ID [ASSIGN exp]{COMMA ID [ASSIGN exp]}
constIdDecl -> CONST type ASSIGN exp {COMMA ID ASSIGN exp}
funcDecl -> FUNCTION ID LSB declParamList RSB COLON type blockedStmt
enumDecl -> ENUM ID LLB ID ASSIGN exp COMMA {ID ASSIGN exp} RLB

exp -> expr
    | exp GE expr
    | exp GT expr
    | exp LE expr
    | exp LT expr
    | exp NE expr
    | exp EQ expr
expr -> expr PLUS term
    | expr MINUS term
    | expr OR term
    | term
term -> term MUL factor
    | term DIV factor
    | term AND factor
    | term MOD factor
    | term XOR factor
    | factor
factor -> literal
    | ID
    | call
    | LSB exp RSB
    | NOT factor
    | MINUS factor
    | ID DOT ID
    | ID DOT call
    | ID LMB exp RMB
    | LLB literal {COMMA literal} RLB
    | SIZEOF LSB type RSB
call -> ID LSB [paramList] RSB
literal -> INTEGER | BOOL | STR | DNUMBER | FNUMBER
    | CHARACTER
paramList -> exp {COMMA exp}
declParamList -> type ID {COMMA type ID}

def -> classDef
classDef -> CLASS ID [COLON ID] LLB {[scope] classCompoDef} RLB
classCompoDef -> funcDef | idDecl
scope -> PUBLIC | PRIVATE | PROTECTED

ifStmt -> IF LSB exp RSB oneStmt [elseBlock]
elseBlock -> ELSE oneStmt

forStmt -> FOR LSB exp SEMI exp SEMI exp RSB oneStmt
    | FOR LSB type ID IN exp RSB oneStmt

whileStmt -> WHILE LSB exp RSB oneStmt
switchStmt -> SWITCH LSB exp RSB LLB {caseStmt} RLB
caseStmt -> CASE exp COLON {oneStmt}
    | DEFAULT COLON {oneStmt}

retStmt -> RETURN exp

IOStmt -> READ LSB STRING COMMA ID {COMMA ID} RSB
    | PRINT LSB STRING COMMA exp {COMMA exp} RSB

type -> INT | CHAR | DOUBLE | FLOAT | BOOLEAN | VOID
    | STRING | ID | LMB RMB type
    | FUNCTION LSB [type ID {COMMA type ID}] RSB COLON type

throwStmt -> THROW exp

tryStmt -> TRY LLB {oneStmt} RLB CATCH LSB type ID RSB LLB {oneStmt} RLB

assign -> ID ASSIGN exp
    | ID DOT ID ASSIGN exp
    | ID LMB exp RMB ASSIGN exp
```
