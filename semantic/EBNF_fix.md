```enbf
program -> {stmt}

stmt -> decl SEMI | ifStmt | forStmt
    | whileStmt | doWhileStmt | BREAK SEMI | CONTINUE SEMI

whileStmt -> WHILE LSB exp RSB oneStmt

doWhileStmt -> DO blockedStmt WHILE LSB exp RSB

nullableStmt -> decl | exp |

ifStmt -> IF LSB exp RSB blockedStmt | IF LSB exp RSB blockedStmt ELSE blockedStmt | IF LSB exp RSB blockedStmt ELSE ifStmt

forStmt -> FOR LSB nullableStmt SEMI exp SEMI nullableStmt RSB blockedStmt

blockedStmt -> LLB stmts RLB | LLB RLB

decl -> idDecl | constIdDecl | funcDecl | enumDecl

idDecl -> type id | type id [ASSIGN exp]{COMMA ID [ASSIGN exp]}

literalArray -> LLB literalList RLB

literalList -> literalList COMMA literal | literal

constIdDecl -> CONST type id | CONST type ASSIGN exp {COMMA ID ASSIGN exp}

funcDecl -> FUNCTION ID LSB declParamList RSB COLON type blockedStmt

declParamList -> idDecl | constIdDecl | declParamList COMMA constIdDecl | declParamList COMMA idDecl | 

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
    | id
    | call
    | assign
    | LSB exp RSB
    | NOT factor
    | MINUS factor
    | INC id
    | DEC id
    | id INC
    | id DEC
    | id DOT id
    | id DOT call
    | id arrayIndices
    | arrayDimensions type literalArray
    | NEW arrayDimensions type LSB RSB
    | SIZEOF LSB type RSB    

arrayDimensions -> arrayDimensions LMB INTEGER RMB
							| LMB INTEGER RMB

call -> id LSB RSB | id LSB paramList RSB

assign -> id ASSIGN exp
		| id arrayIndices ASSIGN exp
    | id DOT ID ASSIGN exp

paramList -> exp | paramList COMMA exp

literal -> INTEGER | BOOL | STR | DNUMBER | FNUMBER
    | CHARACTER
    
type -> id | basicType | arrayDimensions basic Type

basicType -> INT | CHAR | DOUBLE | FLOAT | BOOLEAN | VOID | STRING

id -> ID

```

