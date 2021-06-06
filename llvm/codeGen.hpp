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

#define PRINT(s) std::cout << "\n-------\n";s->print(llvm::outs());std::cout << "\n-------\n";

class NBlock;

class VariableRecord {
public:
    llvm::Value *value;
    llvm::Type *dType;
    std::vector<uint32_t> *size;

    VariableRecord(llvm::Value *value, llvm::Type *dType, std::vector<uint32_t> *size) :
            value(value), dType(dType), size(size) {}
};

class LoopInfo {
public:
    NExpression *cond = nullptr;
    NStatement *inc = nullptr;
    llvm::BasicBlock *loop = nullptr;
    llvm::BasicBlock *after = nullptr;

    LoopInfo(NExpression *cond, NStatement *inc, llvm::BasicBlock *loop, llvm::BasicBlock *after) :
            cond(cond), inc(inc), loop(loop), after(after) {}
};

class ActiveRecord {
public:
    llvm::BasicBlock *block = nullptr;
    llvm::Value *retVal = nullptr;
    std::map<std::string, VariableRecord *> localVal;
    LoopInfo *info = nullptr;

    explicit ActiveRecord(llvm::BasicBlock *block, llvm::Value *retVal = nullptr, LoopInfo *info = nullptr) : block(
            block), retVal(retVal), info(info) {}
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

    auto *current() { return arStack.back(); }

    LoopInfo *currentLoop() {
        for (auto it = arStack.rbegin(); it != arStack.rend(); it++) {
            if ((**it).info) {
                return (**it).info;
            }
        }
        return nullptr;
    }

    void push(llvm::BasicBlock *block) {
        arStack.push_back(new ActiveRecord(block));
    }

    void push(llvm::BasicBlock *block, LoopInfo *info) {
        arStack.push_back(new ActiveRecord(block, nullptr, info));
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
            return llvm::PointerType::getInt8PtrTy(llvmContext);
//            return llvm::ArrayType::get(typeOf("char"), 0);
        } else return llvm::Type::getVoidTy(llvmContext);
    }

    llvm::Value *castToBoolean(llvm::Value *value) {
        if (value->getType()->isIntegerTy()) {
            return builder.CreateICmpNE(value, builder.CreateIntCast(
                    builder.getInt1(false), value->getType(), false));
        } else if (value->getType()->isFloatTy() || value->getType()->isDoubleTy()) {
            return builder.CreateFCmpONE(value, llvm::ConstantFP::get(llvmContext, llvm::APFloat(0.0)));
        } else return builder.getInt1(true);
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
    builder.CreateRetVoid();
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

int NExpression::getDType() {
    return -1;
}

llvm::Value *NInteger::codeGen(ARStack &context) {
    return llvm::ConstantInt::get(context.typeOf("int"), value, true);
}

int NInteger::getDType() {
    return INTEGER;
}

llvm::Value *NFloat::codeGen(ARStack &context) {
    return llvm::ConstantFP::get(context.typeOf("float"), value);
}

int NFloat::getDType() {
    return FNUMBER;
}

llvm::Value *NDouble::codeGen(ARStack &context) {
    return llvm::ConstantFP::get(context.typeOf("double"), value);
}

int NDouble::getDType() {
    return DNUMBER;
}

llvm::Value *NBoolean::codeGen(ARStack &context) {
    return llvm::ConstantInt::get(context.typeOf("boolean"), value, false);
}

int NBoolean::getDType() {
    return BOOL;
}

llvm::Value *NChar::codeGen(ARStack &context) {
    return llvm::ConstantInt::get(context.typeOf("char"), value, false);
}

int NChar::getDType() {
    return CHARACTER;
}

llvm::Value *NString::codeGen(ARStack &context) {
    static int i = 0;
    auto charType = context.typeOf("char");

    std::vector<llvm::Constant *> str;
    for (auto ch: value) {
        str.push_back(llvm::ConstantInt::get(charType, (uint8_t)ch));
    }
    str.push_back(llvm::ConstantInt::get(charType, '\0'));

    auto stringType = llvm::ArrayType::get(charType, str.size());

    auto globalDeclaration = (llvm::GlobalVariable *) context.module->getOrInsertGlobal(".str" + std::to_string(i++),
                                                                                        stringType);
    globalDeclaration->setInitializer(llvm::ConstantArray::get(stringType, str));
    globalDeclaration->setConstant(false);
    globalDeclaration->setLinkage(llvm::GlobalValue::LinkageTypes::PrivateLinkage);
    globalDeclaration->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

    return context.builder.CreateBitCast(globalDeclaration, charType->getPointerTo());
}

llvm::Value *NVoid::codeGen(ARStack &context) {
    return llvm::UndefValue::get(context.typeOf("void"));
}

llvm::Value *NArray::codeGen(ARStack &context) {
    static int i = 0;
    auto dType = context.typeOf(type->name);
    if (initList) {
        std::vector<llvm::Constant *> arr;
        switch ((*initList->begin())->getDType()) {
            case INTEGER: {
                for (auto ch: *initList) {
                    arr.push_back(llvm::ConstantInt::get(dType, dynamic_cast<NInteger *>(ch)->value));
                }
                break;
            }
            case CHARACTER: {
                for (auto ch: *initList) {
                    arr.push_back(llvm::ConstantInt::get(dType, dynamic_cast<NChar *>(ch)->value));
                }
                break;
            }
            case DNUMBER: {
                for (auto ch: *initList) {
                    arr.push_back(llvm::ConstantFP::get(dType, dynamic_cast<NDouble *>(ch)->value));
                }
                break;
            }
            case FNUMBER: {
                for (auto ch: *initList) {
                    arr.push_back(llvm::ConstantInt::get(dType, dynamic_cast<NFloat *>(ch)->value));
                }
                break;
            }
            case BOOL: {
                for (auto ch: *initList) {
                    arr.push_back(llvm::ConstantInt::get(dType, dynamic_cast<NBoolean *>(ch)->value));
                }
                break;
            }
            default:
                break;
        }
        if (arr.empty()) {
            std::cerr << "Unsupported array initialization!\n";
            return nullptr;
        } else {
            auto arrType = llvm::ArrayType::get(dType, arr.size());

            auto globalDeclaration = (llvm::GlobalVariable *) context.module->getOrInsertGlobal(
                    ".arr" + std::to_string(i++), arrType);
            globalDeclaration->setInitializer(llvm::ConstantArray::get(arrType, arr));
            globalDeclaration->setConstant(false);
            globalDeclaration->setLinkage(llvm::GlobalValue::LinkageTypes::PrivateLinkage);
            globalDeclaration->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
            return context.builder.CreateBitCast(globalDeclaration, dType->getPointerTo());
        }
    } else {
        auto *arrSize = new std::vector<uint32_t>();
        uint64_t size = util::calArrayDim(arrDim, arrSize);
        auto arrType = llvm::ArrayType::get(dType, size);
        auto *val= new llvm::GlobalVariable(*context.module, arrType, false, llvm::GlobalValue::CommonLinkage, 0, "arr");
        auto *constArr = llvm::ConstantAggregateZero::get(arrType);
        val->setInitializer(constArr);
        return context.builder.CreateBitCast(val, dType->getPointerTo());
    }
}

llvm::Value *NBinaryOperator::codeGen(ARStack &context) {
    auto L = lhs->codeGen(context);
    auto R = rhs->codeGen(context);
    bool isFP = false;

    // type upgrade
    if ((!L->getType()->isIntegerTy()) ||
        (!R->getType()->isIntegerTy())) {
        isFP = true;
        if (R->getType()->isIntegerTy()) {
            R = context.builder.CreateUIToFP(R, llvm::Type::getDoubleTy(context.llvmContext), "FTMP");
        }
        if (L->getType()->isIntegerTy()) {
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
            return isFP ? context.builder.CreateFCmpUNE(L, R, "FEQ") : context.builder.CreateICmpNE(L, R, "NE");
        default:
            return nullptr;
    }
}

llvm::Value *NUnaryOperator::codeGen(ARStack &context) {
    auto R = rhs->codeGen(context);
    const bool isFP = R->getType()->isFloatTy() || R->getType()->isDoubleTy();
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
    auto id = context.get(name);
    if (!id || !id->value) {
        std::cerr << "Undeclared value: " << name << std::endl;
        return nullptr;
    }

    if (id->size) {
        return id->value;
    } else return context.builder.CreateLoad(id->value, false, "");
}

llvm::Value *NAssignment::codeGen(ARStack &context) {
    auto id = context.get(lhs.name);
    llvm::Value *res;
    if (!id && !allowDecl) {
        std::cerr << "Undeclared value: " << lhs.name << std::endl;
        return nullptr;
    }
    std::cout << "Creating assignment for " << lhs.name << std::endl;
    auto val = rhs.codeGen(context);
    auto type = val->getType();
    if (type->isArrayTy() || (type->isPointerTy() && type->getPointerElementType()->isArrayTy())) {
        val->setName(lhs.name);
        res = val;
        if (id && !id->value) id->value = val;
        if (id && id->size && *id->size->begin() == 0) {
            (*(id->size))[0] = val->getType()->getArrayNumElements();
        }
    } else {
        if (id) {
            if (id->value) { context.builder.CreateStore(val, id->value); }
            else {
                id->value = val;
            }
            res = id->value;
        } else {
            res = val;
        }
    }
    return res;
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
        auto tmp = new NBinaryOperator(new NInteger(std::to_string(arrDim[i])), MUL, arrayIndices[i - 1]);
        exp = new NBinaryOperator(exp, PLUS, tmp);
    }
    auto idx = exp->codeGen(context);
    std::vector<llvm::Value *> arrV;
    if (!id->value->getType()->isPointerTy())
        arrV.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(context.llvmContext), 0));
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
    return expression->codeGen(context);
}

llvm::Value *NReturnStatement::codeGen(ARStack &context) {
    if(expression) {
        std::cout << "Generating return code for " << typeid(expression).name() << std::endl;
        llvm::Value *retVal = expression->codeGen(context);
        context.setCurrentReturnValue(retVal);
        context.builder.CreateRet(context.getCurrentReturnValue());
        return retVal;
    }
    else {
        return context.builder.CreateRetVoid();
    }

}

llvm::Value *NVariableDeclaration::codeGen(ARStack &context) {
    if (context.current()->localVal.find(id.name) != context.current()->localVal.end()) {
        std::cerr << "Redeclared value: " << id.name << std::endl;
        return nullptr;
    }
    std::cout << "Creating variable declaration " << type.name << " " << id.name << std::endl;
    llvm::Value *alloc = nullptr;
    auto dType = context.typeOf(type.name);
    auto arrDim = type.getArrayDim();

    if (!arrDim && type.name != "string") {
        alloc = context.builder.CreateAlloca(dType, nullptr, id.name);
        context.locals()[id.name] = new VariableRecord(alloc, dType, nullptr);
        if (assignmentExpr != nullptr) {
            (new NAssignment(id, *assignmentExpr))->codeGen(context);
        }
    } else if (arrDim) {
        auto *arrSize = new std::vector<uint32_t>();
        uint64_t size = util::calArrayDim(arrDim, arrSize);
        if (assignmentExpr) {
            alloc = (new NAssignment(id, *assignmentExpr, true))->codeGen(context);
            context.locals()[id.name] = new VariableRecord(alloc, dType, arrSize);
        } else {
            context.locals()[id.name] = new VariableRecord(nullptr, dType, arrSize);
        }
    } else {
        uint32_t size = 0;
        if (assignmentExpr) {
            alloc = (new NAssignment(id, *assignmentExpr, true))->codeGen(context);
        }
        auto sizeV = new std::vector<uint32_t>{size};
        context.locals()[id.name] = new VariableRecord(alloc, dType, sizeV);
    }
    return alloc;
}

llvm::Value *NFunctionDeclaration::codeGen(ARStack &context) {
    std::cout << "Creating function: " << id.name << std::endl;
    std::vector<llvm::Type *> argTypes;
    for (auto item: arguments) {
        auto arrDim = item->type.getArrayDim();
        if (arrDim) {
            argTypes.push_back(context.typeOf(item->type.name)->getPointerTo());
        } else argTypes.push_back(context.typeOf(item->type.name));
    }

    llvm::FunctionType *fType = llvm::FunctionType::get(context.typeOf(type.name), llvm::makeArrayRef(argTypes), false);
    llvm::Function *function = llvm::Function::Create(fType, llvm::GlobalValue::InternalLinkage, id.name,
                                                      context.module);
    llvm::BasicBlock *bBlock = llvm::BasicBlock::Create(context.llvmContext, id.name + "_entry", function, nullptr);
    context.push(bBlock);
    context.builder.SetInsertPoint(bBlock);

    llvm::Function::arg_iterator argsValues = function->arg_begin();
    llvm::Value *argumentValue;

    for (auto item: arguments) {
        auto alloc = item->codeGen(context);
        argumentValue = &*argsValues++;
        argumentValue->setName(item->id.name);

        if (alloc) {
            context.builder.CreateStore(argumentValue, alloc, false);
        } else {
            PRINT(argumentValue);
            context.current()->localVal[item->id.name]->value = argumentValue;
        }
    }

    block.codeGen(context);
    if (type.name == "void") {
        context.builder.CreateRetVoid();
    } else {
        if (context.getCurrentReturnValue() == nullptr) {
            std::cerr << "function needs return value!\n";
            return nullptr;
        }
    }
    context.pop();
    context.builder.SetInsertPoint(context.current()->block);
    context.locals()[id.name] = new VariableRecord(function, fType, nullptr);
    return function;
}

llvm::Value *NFunctionCall::codeGen(ARStack &context) {
    llvm::Function *function = context.module->getFunction(id.name);
    if (function == nullptr) {
        std::cerr << "no such function " << id.name << std::endl;
    }
    std::vector<llvm::Value *> args;

    auto* tmp = new std::vector<uint32_t>();
    bool flag = false;
    for (auto item: params) {
        if(id.name == "scanf" && item != params[0]) {
            auto target = context.get(dynamic_cast<NIdentifier*>(item)->name);
            if (!target->size) {
                context.get(dynamic_cast<NIdentifier*>(item)->name)->size = tmp;
                flag = true;
            }
        }
        auto val = item->codeGen(context);
        if (val->getType()->isArrayTy())
            val = context.builder.CreateBitCast(val, val->getType()->getArrayElementType()->getPointerTo());
        else if (val->getType()->isPointerTy() && val->getType()->getPointerElementType()->isArrayTy()) {
            val = context.builder.CreateBitCast(
                    val, val->getType()->getPointerElementType()->getArrayElementType()->getPointerTo());
        }
        args.push_back(val);
        if(id.name == "scanf" && item != params[0] && flag) {
            flag = false;
            context.get(dynamic_cast<NIdentifier*>(item)->name)->size = nullptr;
        }
    }
    delete tmp;

    std::cout << "Creating method call: " << id.name << std::endl;
    return context.builder.CreateCall(function, args, "");
}

llvm::Value *NArrayElement::codeGen(ARStack &context) {
    auto arr = context.get(id.name);
    PRINT(arr->value)
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
        auto tmp = new NBinaryOperator(new NInteger(std::to_string(arrDim[i])), MUL, arrayIndices[i - 1]);
        exp = new NBinaryOperator(exp, PLUS, tmp);
    }
    auto idx = exp->codeGen(context);
    std::vector<llvm::Value *> arrV;
    if (!arr->value->getType()->isPointerTy())
        arrV.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(context.llvmContext), 0));
    arrV.push_back(idx);
    auto ptr = context.builder.CreateInBoundsGEP(arr->value, llvm::makeArrayRef(arrV), "elementPtr");
    return context.builder.CreateAlignedLoad(ptr, llvm::MaybeAlign(4));
}

llvm::Value *NIfStatement::codeGen(ARStack &context) {
    std::cout << "Creating if statement" << std::endl;
    llvm::Value *condValue = condition->codeGen(context);
    if (!condValue)
        return nullptr;

    condValue = context.castToBoolean(condValue);

    llvm::Function *function = context.builder.GetInsertBlock()->getParent(); // the function where if statement is in

    llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(context.llvmContext, "then", function);
    llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(context.llvmContext, "else");
    llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(context.llvmContext, "afterIf");

    if (elseBlock) {
        context.builder.CreateCondBr(condValue, thenBB, elseBB);
    } else {
        context.builder.CreateCondBr(condValue, thenBB, afterBB);
    }

    context.builder.SetInsertPoint(thenBB);

    context.push(thenBB);

    this->thenBlock->codeGen(context);

    context.pop();

    thenBB = context.builder.GetInsertBlock();

    if (thenBB->getTerminator() == nullptr) {
        context.builder.CreateBr(afterBB);
    }

    if (elseBlock) {
        function->getBasicBlockList().push_back(elseBB);
        context.builder.SetInsertPoint(elseBB);

        context.push(thenBB);

        elseBlock->codeGen(context);

        context.pop();

        context.builder.CreateBr(afterBB);
    }

    function->getBasicBlockList().push_back(afterBB);
    context.builder.SetInsertPoint(afterBB);

    return nullptr;
}

llvm::Value *NForStatement::codeGen(ARStack &context) {
    llvm::Function *function = context.builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *forLoop = llvm::BasicBlock::Create(context.llvmContext, "forLoop", function);
    llvm::BasicBlock *after = llvm::BasicBlock::Create(context.llvmContext, "afterFor");

    if (init)
        init->codeGen(context);

    llvm::Value *condValue = condition->codeGen(context);
    condValue = context.castToBoolean(condValue);

    context.builder.CreateCondBr(condValue, forLoop, after);
    context.builder.SetInsertPoint(forLoop);
    context.push(forLoop, new LoopInfo(condition, inc, forLoop, after));
    block->codeGen(context);
    context.pop();

    if (inc) {
        inc->codeGen(context);
    }

    condValue = condition->codeGen(context);
    condValue = context.castToBoolean(condValue);
    context.builder.CreateCondBr(condValue, forLoop, after);

    function->getBasicBlockList().push_back(after);
    context.builder.SetInsertPoint(after);

    return nullptr;
}

llvm::Value *NWhileStatement::codeGen(ARStack &context) {
    llvm::Function *function = context.builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *whileLoop = llvm::BasicBlock::Create(context.llvmContext, "whileLoop", function);
    llvm::BasicBlock *after = llvm::BasicBlock::Create(context.llvmContext, "afterWhile");

    llvm::Value *condValue = condition->codeGen(context);
    condValue = context.castToBoolean(condValue);

    context.builder.CreateCondBr(condValue, whileLoop, after);
    context.builder.SetInsertPoint(whileLoop);
    context.push(whileLoop, new LoopInfo(condition, nullptr, whileLoop, after));
    block->codeGen(context);
    context.pop();

    condValue = condition->codeGen(context);
    condValue = context.castToBoolean(condValue);
    context.builder.CreateCondBr(condValue, whileLoop, after);

    function->getBasicBlockList().push_back(after);
    context.builder.SetInsertPoint(after);

    return nullptr;
}

llvm::Value *NDoWhileStatement::codeGen(ARStack &context) {
    llvm::Function *function = context.builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *whileLoop = llvm::BasicBlock::Create(context.llvmContext, "doWhileLoop", function);
    llvm::BasicBlock *after = llvm::BasicBlock::Create(context.llvmContext, "afterDoWhile");

    context.builder.SetInsertPoint(whileLoop);
    context.push(whileLoop, new LoopInfo(condition, nullptr, whileLoop, after));
    block->codeGen(context);
    context.pop();

    llvm::Value *condValue = condition->codeGen(context);
    condValue = context.castToBoolean(condValue);
    context.builder.CreateCondBr(condValue, whileLoop, after);

    function->getBasicBlockList().push_back(after);
    context.builder.SetInsertPoint(after);

    return nullptr;
}

llvm::Value *NBreakStatement::codeGen(ARStack &context) {
    LoopInfo *info = context.currentLoop();
    if (info) {
        context.builder.CreateBr(info->after);
    } else {
        std::cerr << "Use break outside loop!\n";
    }
    return nullptr;
}

llvm::Value *NContinueStatement::codeGen(ARStack &context) {
    LoopInfo *info = context.currentLoop();
    if (info) {
        if (info->inc) {
            info->inc->codeGen(context);
        }
        auto cond = info->cond->codeGen(context);
        cond->print(llvm::outs());
        context.builder.CreateCondBr(cond, info->loop, info->after);
    } else {
        std::cerr << "Use continue outside loop!\n";
    }
    return nullptr;
}

llvm::Value *NIncOperator::codeGen(ARStack &context) {
    auto R = rhs->codeGen(context);
    auto ptr = context.get(dynamic_cast<NIdentifier*>(rhs)->name)->value;
    auto type = R->getType();
    assert(type->isIntegerTy() || type->isDoubleTy() || type->isFloatTy());
    bool isFP = !type->isIntegerTy();
    auto L = isFP ? llvm::ConstantFP::get(type, 1.0) : llvm::ConstantInt::get(type, 1);
    auto res = isFP ? context.builder.CreateFAdd(L, R, "FINC") : context.builder.CreateAdd(L, R, "INC");

    context.builder.CreateStore(res, ptr);
    return isPrefix ? res : R;
}

llvm::Value *NDecOperator::codeGen(ARStack &context) {
    auto R = rhs->codeGen(context);
    auto ptr = context.get(dynamic_cast<NIdentifier*>(rhs)->name)->value;
    auto type = R->getType();
    assert(type->isIntegerTy() || type->isDoubleTy() || type->isFloatTy());
    bool isFP = !type->isIntegerTy();
    auto L = isFP ? llvm::ConstantFP::get(type, 1.0) : llvm::ConstantInt::get(type, 1);
    auto res = isFP ? context.builder.CreateFSub(R, L, "FDEC") : context.builder.CreateSub(R, L, "DEC");
    context.builder.CreateStore(res, ptr);
    return isPrefix ? res : R;
}

#endif //PHEMIA_CODEGEN_HPP
