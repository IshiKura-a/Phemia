#ifndef PHEMIA_COREFUNC_HPP
#define PHEMIA_COREFUNC_HPP

#include <iostream>
#include "codeGen.hpp"
#include "node.h"

extern int yyparse();
extern NBlock* programBlock;

void createPrintf(ARStack& context) {
    std::vector<llvm::Type*> argTypes;
    argTypes.push_back(context.typeOf("string"));
    auto fType = llvm::FunctionType::get(
            context.typeOf("int"), argTypes, true);
    auto printf = llvm::Function::Create(
            fType, llvm::Function::ExternalLinkage,
            llvm::Twine("printf"),
            context.module
    );
    printf->setCallingConv(llvm::CallingConv::C);
}

void createScanf(ARStack& context) {
    std::vector<llvm::Type*> argTypes;
    argTypes.push_back(context.typeOf("string"));
    auto fType = llvm::FunctionType::get(
            context.typeOf("int"), argTypes, true);
    auto printf = llvm::Function::Create(
            fType, llvm::Function::ExternalLinkage,
            llvm::Twine("scanf"),
            context.module
    );
    printf->setCallingConv(llvm::CallingConv::C);
}
void createCoreFunction(ARStack& context) {
    createPrintf(context);
    createScanf(context);
}
#endif //PHEMIA_COREFUNC_HPP
