#ifndef PHEMIA_CODEGEN_HPP
#define PHEMIA_CODEGEN_HPP

#include <stack>
#include <string>
#include <typeinfo>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <regex>

#include "node.h"
#include "parser.hpp"
#include "util.hpp"

class NBlock;

static llvm::LLVMContext staticContext;

class ActiveRecord {
public:
    llvm::BasicBlock *block = nullptr;
    llvm::Value *retVal = nullptr;
    std::map<std::string, llvm::Value *> localVal;

    ActiveRecord(llvm::BasicBlock *block, llvm::Value *retVal) : block(block), retVal(retVal) {}
};

class ARStack {
    std::stack<ActiveRecord *> arStack;
    llvm::Function *main = nullptr;

public:
    llvm::Module *module;

    ARStack() { module = new llvm::Module("main", staticContext); }

    void generateCode(NBlock &root, const std::string& bcFile);

    llvm::GenericValue runCode();

    std::map<std::string, llvm::Value *> &locals() { return arStack.top()->localVal; }

    llvm::BasicBlock *current() { return arStack.top()->block; }

    void push(llvm::BasicBlock *block) {
        arStack.push(new ActiveRecord(block, nullptr));
    }

    void pop() {
        ActiveRecord *top = arStack.top();
        arStack.pop();
        delete top;
    }

    void setCurrentReturnValue(llvm::Value *value) { arStack.top()->retVal = value; }

    llvm::Value *getCurrentReturnValue() { return arStack.top()->retVal; }
};

void ARStack::generateCode(NBlock &root, const std::string& bcFile) {
    /* Create the top level interpreter function to call as entry */
    std::vector<llvm::Type*> argTypes;
    llvm::FunctionType *fType = llvm::FunctionType::get(llvm::Type::getInt32Ty(staticContext), llvm::makeArrayRef(argTypes), false);
    main = llvm::Function::Create(fType, llvm::GlobalValue::ExternalLinkage, "main", module);
    llvm::BasicBlock *bBlock = llvm::BasicBlock::Create(staticContext, "entry", main, nullptr);

    /* Push a new variable/block context */
    push(bBlock);
    root.codeGen(*this); /* emit bytecode for the toplevel block */
    llvm::ReturnInst::Create(staticContext, llvm::ConstantInt::get(llvm::Type::getInt32Ty(staticContext), 0), bBlock);
    pop();

    /* Print the bytecode in a human-readable format to see if our program compiled properly */
    llvm::legacy::PassManager pm;
    pm.add(llvm::createPrintModulePass(llvm::outs()));
    pm.run(*module);

    std::error_code errInfo;
    llvm::raw_ostream *out = new llvm::raw_fd_ostream(bcFile, errInfo);
    llvm::WriteBitcodeToFile(*module, *out);
    out->flush();
    delete out;
}

llvm::GenericValue ARStack::runCode() {
    llvm::ExecutionEngine *ee = llvm::EngineBuilder(std::unique_ptr<llvm::Module>(module)).create();
    ee->finalizeObject();
    std::vector <llvm::GenericValue> noArgs;
    llvm::GenericValue v = ee->runFunction(main, noArgs);
    return v;
}

static llvm::Type *typeOf(const std::string &type, std::vector<int> size, llvm::LLVMContext &context = staticContext) {
    std::smatch result;
    if (type == "int") {
        return llvm::Type::getInt32Ty(context);
    } else if (type == "float") {
        return llvm::Type::getFloatTy(context);
    } else if (type == "double") {
        return llvm::Type::getDoubleTy(context);
    } else if (type == "char") {
        return llvm::Type::getInt8Ty(context);
    } else if (type == "boolean") {
        return llvm::Type::getInt1Ty(context);
    } else if (type == "string") {
        assert(size.size() == 1);
        return llvm::ArrayType::get(llvm::Type::getInt8Ty(context), size[0]);
    } else if (std::regex_match(type, result, std::regex(R"(\[\](\w+))"))) {
        assert(!size.empty());
        const int len = size.back();
        size.pop_back();
        return llvm::ArrayType::get(typeOf(result[1], size), len);
    } else return llvm::Type::getVoidTy(context);
}

llvm::Value *NInteger::codeGen(ARStack &context) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(staticContext), value, true);
}

llvm::Value *NFloat::codeGen(ARStack &context) {
    return llvm::ConstantFP::get(llvm::Type::getFloatTy(staticContext), value);
}

llvm::Value *NDouble::codeGen(ARStack &context) {
    return llvm::ConstantFP::get(llvm::Type::getDoubleTy(staticContext), value);
}

llvm::Value *NBoolean::codeGen(ARStack &context) {
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(staticContext), value, true);
}

llvm::Value *NChar::codeGen(ARStack &context) {
    return llvm::ConstantInt::get(llvm::Type::getInt8Ty(staticContext), value, true);
}

llvm::Value *NString::codeGen(ARStack &context) {
    std::vector<llvm::Constant *> values;
    for (const char ch: value) {
        llvm::Constant *c = llvm::Constant::getIntegerValue(llvm::Type::getInt8Ty(staticContext), llvm::APInt(8, ch));
        values.push_back(c);
    }
    return llvm::ConstantArray::get(llvm::ArrayType::get(llvm::Type::getInt8Ty(staticContext), value.size()), values);
}

llvm::Value *NVoid::codeGen(ARStack &context) {
    return llvm::UndefValue::get(llvm::Type::getVoidTy(staticContext));
}

llvm::Value *NArray::codeGen(ARStack &context) {
    return nullptr;
}

llvm::Value *NBinaryOperator::codeGen(ARStack &context) {
    llvm::Instruction::BinaryOps inst;
    const bool isFP = (util::instanceof<NFloat>(lhs)
                       || util::instanceof<NFloat>(rhs)
                       || util::instanceof<NDouble>(lhs)
                       || util::instanceof<NDouble>(rhs));
    switch (op) {
        case PLUS:
            inst = llvm::Instruction::Add;
            break;
        case MINUS:
            inst = llvm::Instruction::Sub;
            break;
        case MUL:
            inst = llvm::Instruction::Mul;
            break;
        case DIV:
            inst = isFP ? llvm::Instruction::FDiv : llvm::Instruction::SDiv;
            break;
        case MOD:
            inst = llvm::Instruction::URem;
            break;
        case AND:
            inst = llvm::Instruction::And;
            break;
        case OR:
            inst = llvm::Instruction::Or;
            break;
        case XOR:
            inst = llvm::Instruction::Xor;
            break;
        default:
            return nullptr;
    }
    return llvm::BinaryOperator::Create(inst, lhs.codeGen(context),
                                        rhs.codeGen(context), "", context.current());
}

llvm::Value *NUnaryOperator::codeGen(ARStack &context) {
    llvm::Instruction::UnaryOps inst;
    const bool isFP = (util::instanceof<NFloat>(rhs)
                       || util::instanceof<NDouble>(rhs));
    switch (op) {
        case NOT:
            inst = llvm::Instruction::FNeg; break;
        case MINUS:
            inst = llvm::Instruction::FNeg; break;
        default: return nullptr;
    }
    return llvm::UnaryOperator::Create(inst, rhs.codeGen(context), "", context.current());
}

llvm::Value *NIdentifier::codeGen(ARStack &context) {
    if (context.locals().find(name) == context.locals().end()) {
        std::cerr << "Undeclared value: " << name << std::endl;
    }
    return new llvm::LoadInst(context.locals()[name]->getType(),context.locals()[name], "", false, context.current());
}

llvm::Value *NAssignment::codeGen(ARStack &context) {
    std::cout << "Creating assignment for " << lhs.name << std::endl;
    if (context.locals().find(lhs.name) == context.locals().end()) {
        std::cerr << "Undeclared value: " << lhs.name << std::endl;
    }
    return new llvm::StoreInst(rhs.codeGen(context), context.locals()[lhs.name], false, context.current());
}

llvm::Value *NArrayAssignment::codeGen(ARStack &context) {
    return nullptr;
}

llvm::Value *NClassAssignment::codeGen(ARStack &context) {
    return nullptr;
}

llvm::Value *NBlock::codeGen(ARStack &context) {
    StatementList::const_iterator it;
    llvm::Value *last = nullptr;
    for (it = statements.begin(); it != statements.end(); it++) {
        auto& statement = **it;
        std::cout << "Generating code for " << typeid(statement).name() << std::endl;
        last = (statement).codeGen(context);
    }
    std::cout << "Creating block" << std::endl;
    return last;
}

llvm::Value *NExpressionStatement::codeGen(ARStack &context) {
    std::cout << "Generating code for " << typeid(expression).name() << std::endl;
    return expression.codeGen(context);
}

llvm::Value *NReturnStatement::codeGen(ARStack &context) {
    std::cout << "Generating return code for " << typeid(expression).name() << std::endl;
    llvm::Value *retVal = expression.codeGen(context);
    context.setCurrentReturnValue(retVal);
    return retVal;
}

llvm::Value *NVariableDeclaration::codeGen(ARStack &context) {
    std::cout << "Creating variable declaration " << type.name << " " << id.name << std::endl;
    auto *alloc = new llvm::AllocaInst(typeOf(type.name, {}), 0, id.name, context.current());
    context.locals()[id.name] = alloc;
    if (assignmentExpr != nullptr) {
        NAssignment assignment(id, *assignmentExpr);
        assignment.codeGen(context);
    }
    return alloc;
}

llvm::Value *NFunctionDeclaration::codeGen(ARStack &context) {
    std::vector<llvm::Type *> argTypes;
    for (auto item: arguments) {
        argTypes.push_back(typeOf(item->type.name, {}));
    }

    llvm::FunctionType *fType = llvm::FunctionType::get(typeOf(type.name, {}), llvm::makeArrayRef(argTypes), false);
    llvm::Function *function = llvm::Function::Create(fType, llvm::GlobalValue::InternalLinkage, id.name, context.module);
    llvm::BasicBlock *bBlock = llvm::BasicBlock::Create(staticContext, "entry", function, nullptr);

    context.push(bBlock);

    llvm::Function::arg_iterator argsValues = function->arg_begin();
    llvm::Value* argumentValue;

    for (auto item: arguments) {
        item->codeGen(context);
        argumentValue = &*argsValues++;
        argumentValue->setName(item->id.name);
    }

    block.codeGen(context);
    llvm::ReturnInst::Create(staticContext, context.getCurrentReturnValue(), bBlock);
    context.pop();
    return function;
}
#endif //PHEMIA_CODEGEN_HPP
