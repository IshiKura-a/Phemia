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

class VariableRecord {
public:
    llvm::Value *value;
    llvm::Type *dType;
    std::vector<uint32_t> *size;

    VariableRecord(llvm::Value *value, llvm::Type *dType, std::vector<uint32_t> *size) :
            value(value), dType(dType), size(size) {}
};

class ActiveRecord {
public:
    llvm::BasicBlock *block = nullptr;
    llvm::Value *retVal = nullptr;
    std::map<std::string, VariableRecord *> localVal;

    ActiveRecord(llvm::BasicBlock *block, llvm::Value *retVal) : block(block), retVal(retVal) {}
};


class ARStack {
    std::vector<ActiveRecord *> arStack;
    llvm::Function *main = nullptr;
public:
    llvm::LLVMContext llvmContext;
    llvm::IRBuilder<> builder;
    llvm::Module *module;

    ARStack() : builder(llvmContext) { module = new llvm::Module("main", llvmContext); }

    void generateCode(NBlock &root, const std::string &bcFile);

    llvm::GenericValue runCode();

    std::map<std::string, VariableRecord *> &locals() { return arStack.back()->localVal; }

    VariableRecord *get(const std::string &name) const {
        for (auto it = arStack.rbegin(); it != arStack.rend(); it++) {
            if ((*it)->localVal.find(name) != (*it)->localVal.end()) {
                return (*it)->localVal[name];
            }
        }
        return nullptr;
    }

    llvm::BasicBlock *current() { return arStack.back()->block; }

    void push(llvm::BasicBlock *block) {
        arStack.push_back(new ActiveRecord(block, nullptr));
    }

    void pop() {
        ActiveRecord *top = arStack.back();
        arStack.pop_back();
        delete top;
    }

    void setCurrentReturnValue(llvm::Value *value) { arStack.back()->retVal = value; }

    llvm::Value *getCurrentReturnValue() { return arStack.back()->retVal; }

    llvm::Type *typeOf(const std::string &type) {
        std::smatch result;
        if (type == "int") {
            return llvm::Type::getInt32Ty(llvmContext);
        } else if (type == "float") {
            return llvm::Type::getFloatTy(llvmContext);
        } else if (type == "double") {
            return llvm::Type::getDoubleTy(llvmContext);
        } else if (type == "char") {
            return llvm::Type::getInt8Ty(llvmContext);
        } else if (type == "boolean") {
            return llvm::Type::getInt1Ty(llvmContext);
        } else if (type == "string") {
            return llvm::PointerType::getInt8Ty(llvmContext);
        } else return llvm::Type::getVoidTy(llvmContext);
    }
};

void ARStack::generateCode(NBlock &root, const std::string &bcFile) {
    /* Create the top level interpreter function to call as entry */
    std::vector<llvm::Type *> argTypes;
    llvm::FunctionType *fType = llvm::FunctionType::get(llvm::Type::getVoidTy(llvmContext),
                                                        llvm::makeArrayRef(argTypes), false);
    main = llvm::Function::Create(fType, llvm::GlobalValue::ExternalLinkage, "main", module);
    llvm::BasicBlock *bBlock = llvm::BasicBlock::Create(llvmContext, "entry", main, nullptr);
    builder.SetInsertPoint(bBlock);
    /* Push a new variable/block context */
    push(bBlock);
    root.codeGen(*this); /* emit bytecode for the toplevel block */
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
    std::vector<llvm::GenericValue> noArgs;
    llvm::GenericValue v = ee->runFunction(main, noArgs);
    return v;
}

llvm::Value *NInteger::codeGen(ARStack &context) {
    return llvm::ConstantInt::get(context.typeOf("int"), value, true);
}

llvm::Value *NFloat::codeGen(ARStack &context) {
    return llvm::ConstantFP::get(context.typeOf("float"), value);
}

llvm::Value *NDouble::codeGen(ARStack &context) {
    return llvm::ConstantFP::get(context.typeOf("double"), value);
}

llvm::Value *NBoolean::codeGen(ARStack &context) {
    return llvm::ConstantInt::get(context.typeOf("boolean"), value, false);
}

llvm::Value *NChar::codeGen(ARStack &context) {
    return llvm::ConstantInt::get(context.typeOf("char"), value, false);
}

llvm::Value *NString::codeGen(ARStack &context) {
    auto charType = context.typeOf("char");

    std::vector<llvm::Constant *> str;
    for (auto ch: value) {
        str.push_back(llvm::ConstantInt::get(charType, ch));
    }

    auto stringType = llvm::ArrayType::get(charType, str.size());

    auto globalDeclaration = (llvm::GlobalVariable *) context.module->getOrInsertGlobal(".str", stringType);
    globalDeclaration->setInitializer(llvm::ConstantArray::get(stringType, str));
    globalDeclaration->setConstant(false);
    globalDeclaration->setLinkage(llvm::GlobalValue::LinkageTypes::PrivateLinkage);
    globalDeclaration->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

    // Return a cast to an i8*
    return llvm::ConstantExpr::getBitCast(globalDeclaration, charType->getPointerTo());
}

llvm::Value *NVoid::codeGen(ARStack &context) {
    return llvm::UndefValue::get(context.typeOf("void"));
}

llvm::Value *NArray::codeGen(ARStack &context) {
    auto dType = context.typeOf(type.name);
    auto dSize = llvm::ConstantInt::get(context.typeOf("int"),
                                        llvm::DataLayout(context.module).getTypeAllocSize(dType));
    auto size = llvm::ConstantInt::get(context.typeOf("int"), std::strtol(arrDim.back()->c_str(), nullptr, 10));
    auto allocSize = llvm::ConstantExpr::getMul(dSize, size);

    // malloc:
    auto arr = llvm::CallInst::CreateMalloc(
            context.builder.GetInsertBlock(),
            dType->getPointerTo(),
            dType,
            allocSize,
            nullptr,
            nullptr,
            "");
    return context.builder.Insert(arr);
}

llvm::Value *NBinaryOperator::codeGen(ARStack &context) {
    auto L = lhs.codeGen(context);
    auto R = rhs.codeGen(context);
    bool isFP = false;

    // type upgrade
    if ((L->getType()->getTypeID() == llvm::Type::DoubleTyID) ||
        (R->getType()->getTypeID() == llvm::Type::DoubleTyID)) {
        isFP = true;
        if ((R->getType()->getTypeID() != llvm::Type::DoubleTyID)) {
            R = context.builder.CreateUIToFP(R, llvm::Type::getDoubleTy(context.llvmContext), "FTMP");
        }
        if ((L->getType()->getTypeID() != llvm::Type::DoubleTyID)) {
            L = context.builder.CreateUIToFP(L, llvm::Type::getDoubleTy(context.llvmContext), "FTMP");
        }
    }

    if (!L || !R) {
        return nullptr;
    }

    switch (op) {
        case PLUS:
            return isFP ? context.builder.CreateFAdd(L, R, "FPLUS") : context.builder.CreateAdd(L, R, "PLUS");
        case MINUS:
            return isFP ? context.builder.CreateFSub(L, R, "FMINUS") : context.builder.CreateSub(L, R, "MINUS");
        case MUL:
            return isFP ? context.builder.CreateFMul(L, R, "FMUL") : context.builder.CreateMul(L, R, "MUL");
        case DIV:
            return isFP ? context.builder.CreateFDiv(L, R, "FDIV") : context.builder.CreateSDiv(L, R, "DIV");
        case MOD:
            return isFP ? context.builder.CreateFRem(L, R, "FMOD") : context.builder.CreateSRem(L, R, "MOD");
        case AND:
            if (isFP) std::cerr << "Compute AND on FP!\n";
            return isFP ? nullptr : context.builder.CreateAnd(L, R, "AND");
        case OR:
            if (isFP) std::cerr << "Compute OR on FP!\n";
            return isFP ? nullptr : context.builder.CreateOr(L, R, "OR");
        case XOR:
            if (isFP) std::cerr << "Compute XOR on FP!\n";
            return isFP ? nullptr : context.builder.CreateXor(L, R, "XOR");
        case LT:
            return isFP ? context.builder.CreateFCmpULT(L, R, "FLT") : context.builder.CreateICmpULT(L, R, "LT");
        case LE:
            return isFP ? context.builder.CreateFCmpULE(L, R, "FLE") : context.builder.CreateICmpULE(L, R, "LE");
        case GT:
            return isFP ? context.builder.CreateFCmpUGT(L, R, "FGT") : context.builder.CreateICmpUGT(L, R, "GT");
        case GE:
            return isFP ? context.builder.CreateFCmpUGE(L, R, "FGE") : context.builder.CreateICmpUGE(L, R, "GE");
        case EQ:
            return isFP ? context.builder.CreateFCmpUEQ(L, R, "FEQ") : context.builder.CreateICmpEQ(L, R, "EQ");
        case NE:
            return isFP ? context.builder.CreateFCmpUNE(L, R, "FEQ") : context.builder.CreateICmpNE(L, R, "EQ");
        default:
            return nullptr;
    }
}

llvm::Value *NUnaryOperator::codeGen(ARStack &context) {
    llvm::Instruction::UnaryOps inst;
    const bool isFP = (util::instanceof<NFloat>(rhs)
                       || util::instanceof<NDouble>(rhs));
    auto R = rhs.codeGen(context);
    switch (op) {
        case NOT:
            if (isFP) std::cerr << "Compute NOT on FP!\n";
            return context.builder.CreateNot(R, "NOT");
        case MINUS:
            return isFP ? context.builder.CreateFNeg(R, "FNEG") : context.builder.CreateNeg(R, "NEG");
        default:
            return nullptr;
    }
}

llvm::Value *NIdentifier::codeGen(ARStack &context) {
    auto id = context.get(name)->value;
    if (!id) {
        std::cerr << "Undeclared value: " << name << std::endl;
        return nullptr;
    }

    return context.builder.CreateLoad(id, false, "");
}

llvm::Value *NAssignment::codeGen(ARStack &context) {
    auto id = context.get(lhs.name);
    if (!id) {
        std::cerr << "Undeclared value: " << lhs.name << std::endl;
        return nullptr;
    }
    std::cout << "Creating assignment for " << lhs.name << std::endl;
    auto val = rhs.codeGen(context);
    context.builder.CreateStore(val, id->value);
    return id->value;
}

llvm::Value *NArrayAssignment::codeGen(ARStack &context) {
    auto id = context.get(lhs.name);
    if (!id) {
        std::cerr << "Undeclared value: " << lhs.name << std::endl;
        return nullptr;
    }
    if (!id->size) {
        std::cerr << "Unindexable value: " << lhs.name << std::endl;
        return nullptr;
    }
    auto val = rhs.codeGen(context);
    if (id->dType->getTypeID() != val->getType()->getTypeID()) {
        std::cerr << "Cannot assign ";
        val->getType()->print(llvm::errs());
        std::cerr << " to ";
        id->dType->print(llvm::errs());
        std::cerr << std::endl;
        return nullptr;
    }

    assert(arrayIndices.size() == id->size->size());
    auto arrDim = *(id->size);
    auto exp = *(arrayIndices.rbegin());
    for (unsigned i = arrayIndices.size() - 1; i >= 1; i--) {
        auto tmp = new NBinaryOperator(*(new NInteger(std::to_string(arrDim[i]))), MUL, *arrayIndices[i - 1]);
        exp = new NBinaryOperator(*exp, PLUS, *tmp);
    }
    auto idx = exp->codeGen(context);
    std::vector<llvm::Value *> arrV;
//    arrV.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(context.llvmContext), 0));
    arrV.push_back(idx);
    auto ptr = context.builder.CreateInBoundsGEP(id->value, llvm::makeArrayRef(arrV), "elementPtr");
    return context.builder.CreateAlignedStore(val, ptr, llvm::MaybeAlign(4));
}

llvm::Value *NClassAssignment::codeGen(ARStack &context) {
    // TODO: class
    return nullptr;
}

llvm::Value *NBlock::codeGen(ARStack &context) {
    StatementList::const_iterator it;
    llvm::Value *last = nullptr;
    for (it = statements.begin(); it != statements.end(); it++) {
        auto &statement = **it;
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
    llvm::Value *alloc = nullptr;
    auto dType = context.typeOf(type.name);
    if (util::instanceof<NArrayType>(type)) {
        uint64_t size = 1;
        auto *arrSize = new std::vector<uint32_t>();
        for (auto dim: dynamic_cast<const NArrayType &>(type).arrDim) {
            uint32_t n = strtol(dim->c_str(), nullptr, 10);
            size *= n;
            arrSize->push_back(n);
        }
        llvm::Value *sizeValue = NInteger(std::to_string(size)).codeGen(context);
        auto arrType = llvm::ArrayType::get(dType, size);
        alloc = context.builder.CreateAlloca(arrType, sizeValue, id.name);
        context.locals()[id.name] = new VariableRecord(alloc, dType, arrSize);
    } else if (type.name == "string") {
        uint32_t size = assignmentExpr ? dynamic_cast<NString *>(assignmentExpr)->value.size(): 0;
        auto sizeV = new std::vector<uint32_t>{size};
        alloc = context.builder.CreateAlloca(dType->getPointerTo(), context.builder.getInt32(size), id.name);
        context.locals()[id.name] = new VariableRecord(alloc, dType, sizeV);
    } else {
        alloc = context.builder.CreateAlloca(dType, nullptr, id.name);
        context.locals()[id.name] = new VariableRecord(alloc, dType, nullptr);
    }
    if (assignmentExpr != nullptr) {
        (new NAssignment(id, *assignmentExpr))->codeGen(context);
    }
    return alloc;
}

llvm::Value *NFunctionDeclaration::codeGen(ARStack &context) {
    std::cout << "Creating function: " << id.name << std::endl;
    std::vector<llvm::Type *> argTypes;
    for (auto item: arguments) {
        // TODO: array
        argTypes.push_back(context.typeOf(item->type.name));
    }

    llvm::FunctionType *fType = llvm::FunctionType::get(context.typeOf(type.name), llvm::makeArrayRef(argTypes), false);
    llvm::Function *function = llvm::Function::Create(fType, llvm::GlobalValue::InternalLinkage, id.name,
                                                      context.module);
    llvm::BasicBlock *bBlock = llvm::BasicBlock::Create(context.llvmContext, "entry", function, nullptr);
    context.builder.SetInsertPoint(bBlock);
    context.push(bBlock);

    llvm::Function::arg_iterator argsValues = function->arg_begin();
    llvm::Value *argumentValue;

    for (auto item: arguments) {
        auto alloc = item->codeGen(context);
        argumentValue = &*argsValues++;
        argumentValue->setName(item->id.name);
        context.builder.CreateStore(argumentValue, alloc, false);
    }

    block.codeGen(context);
    context.builder.CreateRet(context.getCurrentReturnValue());
    context.pop();
    context.locals()[id.name] = new VariableRecord(function, fType, nullptr);
    return function;
}

llvm::Value *NFunctionCall::codeGen(ARStack &context) {
    llvm::Function *function = context.module->getFunction(id.name);
    if (function == nullptr) {
        std::cerr << "no such function " << id.name << std::endl;
    }
    std::vector<llvm::Value *> args;
    ExpressionList::const_iterator it;
    for (auto item: params) {
        args.push_back(item->codeGen(context));
    }

    std::cout << "Creating method call: " << id.name << std::endl;
    return context.builder.CreateCall(function, args, "call");
}

llvm::Value *NArrayElement::codeGen(ARStack &context) {
    auto arr = context.get(id.name);
    if (!arr) {
        std::cerr << "Undeclared value: " << id.name << std::endl;
        return nullptr;
    }
    if (!arr->size) {
        std::cerr << "Unindexable value: " << id.name << std::endl;
        return nullptr;
    }

    assert(arrayIndices.size() == arr->size->size());
    auto arrDim = *(arr->size);
    auto exp = *(arrayIndices.rbegin());
    for (unsigned i = arrayIndices.size() - 1; i >= 1; i--) {
        auto tmp = new NBinaryOperator(*(new NInteger(std::to_string(arrDim[i]))), MUL, *arrayIndices[i - 1]);
        exp = new NBinaryOperator(*exp, PLUS, *tmp);
    }
    auto idx = exp->codeGen(context);
    std::vector<llvm::Value *> arrV;
//    arrV.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(context.llvmContext), 0));
    arrV.push_back(idx);
    auto ptr = context.builder.CreateInBoundsGEP(arr->value, llvm::makeArrayRef(arrV), "elementPtr");
    return context.builder.CreateAlignedLoad(ptr, llvm::MaybeAlign(4));
}

llvm::Value *NArrayType::codeGen(ARStack &context) {
    return nullptr;
}

#endif //PHEMIA_CODEGEN_HPP
