```enbf
program -> {oneStmt}

oneStmt -> stmt | blockedStmt
stmt -> decl SEMI | def | exp SEMI | ifStmt | forStmt
    | whileStmt | switchStmt | BREAK SEMI | CONTINUE SEMI
    | retStmt SEMI | IOStmt SEMI | throwStmt SEMI
    | tryStmt
blockedStmt -> LLB {oneStmt} RLB


decl -> idDecl | constIdDecl | funcDecl | enumDecl
idDecl -> type ID [ ASSIGN exp ]{COMMA ID [ ASSIGN exp ]}
constIdDecl -> CONST type ASSIGN exp {COMMA ID ASSIGN exp}
funcDecl -> FUNCTION ID LSB declParamList RSB COLON type blockedStmt
enumDecl -> ENUM ID LLB ID ASSIGN exp COMMA { ID ASSIGN exp } RLB

exp -> [MINUS] signedExp
signedExp -> call
    | signedExp PLUS term
    | signedExp MINUS term
    | NEW ID LSB paramList RSB
    | NEW type LMB INTEGER RMB
    | signedExp DOT ID LSB paramList RSB
    | term
term -> term MUL factor | term DIV factor | factor
call -> ID LSB paramList RSB
factor -> literal | LSB exp RSB | SIZEOF LSB type RSB
    | LLB exp { COMMA exp } RLB
    | ID [ DOT LENGTH ]
literal -> INTEGER | BOOL | WORD | DNUMBER | FNUMBER
    | CHARACTER
paramList -> exp {COMMA exp }
declParamList -> type ID {COMMA type ID }

def -> classDef
classDef -> CLASS ID [COLON ID] LLB {[scope] classCompoDef } RLB
classCompoDef -> funcDef | idDecl
scope -> PUBLIC | PRIVATE | PROTECTED

ifStmt -> IF LSB exp RSB oneStmt [elseStmt]
elseStmt -> ELSE oneStmt

forStmt -> FOR LSB exp SEMI exp SEMI exp RSB oneStmt
    | FOR LSB type ID IN exp RSB oneStmt

whileStmt -> WHILE LSB exp RSB oneStmt
switchStmt -> SWITCH LSB exp RSB LLB { caseStmt } RLB
caseStmt -> CASE exp COLON {oneStmt}
    | DEFAULT COLON {oneStmt}

retStmt -> RETURN exp

IOStmt -> READ LSB STRING COMMA ID {COMMA ID} RSB
    | PRINT LSB STRING COMMA exp { COMMA exp } RSB

type -> INT | CHAR | DOUBLE | FLOAT | BOOLEAN | VOID
    | STRING | ID | LMB RMB type

throwStmt -> THROW exp

tryStmt -> TRY LLB {oneStmt} RLB CATCH LSB type ID RSB LLB {oneStmt} RLB
```
