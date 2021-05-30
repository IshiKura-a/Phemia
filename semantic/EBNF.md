```enbf
program -> {stmtOrMore}

stmtOrMore -> stmt | blockedStmt
stmt -> decl ";" | def | exp ";" | ifStmt | forStmt
    | whileStmt | switchStmt | BREAK ";" | CONTINUE ";"
    | retStmt ";" | IOStmt ";"
blockedStmt -> "{" {stmtOrMore} "}"


decl -> idDecl | funcDecl | enumDecl
idDecl -> type ID [ "=" exp ]{"," ID [ "=" exp ]}
funcDecl -> FUNCTION ID "(" declParamList ")" ":" type blockedStmt
enumDecl -> ENUM ID "{" ID "=" exp "," { ID "=" exp } "}"

exp -> ["-"] signedExp
signedExp -> call
    | signedExp "+" term
    | signedExp "-" term
    | NEW ID "(" paramList ")" {"." ID "(" paramList ")"}
    | ID {"." ID "(" paramList ")"}
term -> term "*" factor | term "/" factor | factor
call -> ID "(" paramList ")"
factor -> NUMBER | "(" exp ")"
paramList -> exp {"," exp }
declParamList -> type ID {"," type ID }

def -> classDef
classDef -> CLASS ID [":" ID] "{" {[scope] classCompoDef }"}"
classCompoDef -> funcDef | idDecl
scope -> PUBLIC | PRIVATE | PROTECTED

ifStmt -> IF "(" exp ")" stmtOrMore [elseStmt]
elseStmt -> ELSE stmtOrMore

forStmt -> FOR "(" exp ";" exp ";" exp ")" stmtOrMore
    | FOR "(" type ID IN exp ")" stmtOrMore

whileStmt -> WHILE "(" exp ")" stmtOrMore
switchStmt -> SWITCH "(" exp ")" "{" { caseStmt } "}"
caseStmt -> CASE exp ":" {stmtOrMore}
    | DEFAULT ":" {stmtOrMore}

retStmt -> RETURN exp

IOStmt -> READ "(" STRING "," ID {"," ID} ")"
    | PRINT "(" STRING , exp { "," exp }")"
```
