#include <iostream>
#include "codeGen.hpp"
#include "coreFunc.hpp"
#include "node.h"

extern FILE *yyin;

extern int yyparse();

extern NBlock *programBlock;

int main(int argc, char **argv) {
    if (argc == 0) {
        std::cerr << "Invalid Param!\n";
        std::exit(1);
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        printf("couldn't open file for reading\n");
        exit(-1);
    }
    yyin = fp;
    int parseErr = yyparse();
    if (parseErr != 0) {
        printf("couldn't complete lex parse\n");
        exit(-1);
    }
    fclose(fp);
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    ARStack context;
    createCoreFunction(context);
    context.generateCode(*programBlock, "test/output.ll");
    return 0;
}
