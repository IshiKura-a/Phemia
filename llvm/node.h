#ifndef PHEMIA_NODE_H
#define PHEMIA_NODE_H

#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <llvm/IR/Value.h>
#include "util.hpp"

class ARStack;

class NStatement;

class NExpression;

class NVariableDeclaration;

typedef std::vector<NStatement *> StatementList;
typedef std::vector<NExpression *> ExpressionList;
typedef std::vector<NVariableDeclaration *> VariableList;
typedef std::vector<std::string *> ArrayDimension;

class Node {
public:
    virtual ~Node() {}

    virtual llvm::Value *codeGen(ARStack &context) { return nullptr; }
};

class NExpression : public Node {
};

class NStatement : public Node {
};

class NIdentifier : public NExpression {
public:
    std::string name;

    NIdentifier(std::string name) : name(std::move(name)) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NInteger : public NExpression {
public:
    int32_t value;

    NInteger(const std::string &value) : value(std::strtol(value.c_str(), nullptr, 10)) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NFloat : public NExpression {
public:
    float value;

    NFloat(const std::string &value) : value(std::strtof(value.c_str(), nullptr)) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NDouble : public NExpression {
public:
    double value;

    NDouble(const std::string &value) : value(std::strtod(value.c_str(), nullptr)) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NBoolean : public NExpression {
public:
    bool value;

    NBoolean(const std::string &value) : value(value == "true") {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NChar : public NExpression {
public:
    char value;

    NChar(const std::string &value) : value(value[0]) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NString : public NExpression {
public:
    std::string value;

    NString(std::string value) : value(std::move(value)) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NVoid : public NExpression {
public:
    NVoid() {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NArray : public NExpression {
public:
    NIdentifier type;
    ArrayDimension arrDim;
    ExpressionList initList;

    NArray(ArrayDimension &arrDim, NIdentifier type, ExpressionList &initList) : arrDim(arrDim),
                                                                                 type(std::move(type)),
                                                                                 initList(initList) {}

    NArray(ArrayDimension &arrDim, NIdentifier type) : arrDim(arrDim), type(std::move(type)) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NBinaryOperator : public NExpression {
public:
    int op;
    NExpression &lhs;
    NExpression &rhs;

    NBinaryOperator(NExpression &lhs, int op, NExpression &rhs) : lhs(lhs), rhs(rhs), op(op) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NUnaryOperator : public NExpression {
public:
    int op;
    NExpression &rhs;

    NUnaryOperator(int op, NExpression &rhs) : op(op), rhs(rhs) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NAssignment : public NExpression {
public:
    NIdentifier &lhs;
    NExpression &rhs;

    NAssignment(NIdentifier &lhs, NExpression &rhs) : lhs(lhs), rhs(rhs) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NClassAssignment : public NAssignment {
public:
    NIdentifier &attribute;

    NClassAssignment(NIdentifier &lhs, NIdentifier &attribute, NExpression &rhs)
            : attribute(attribute), NAssignment(lhs, rhs) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NArrayAssignment : public NAssignment {
public:
    ExpressionList arrayIndices;

    NArrayAssignment(NIdentifier &lhs, ExpressionList &arrayIndices, NExpression &rhs)
            : arrayIndices(arrayIndices), NAssignment(lhs, rhs) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NBlock : public NExpression {
public:
    StatementList statements;

    NBlock() {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NExpressionStatement : public NStatement {
public:
    NExpression *expression;

    NExpressionStatement(NExpression *expression = nullptr) : expression(expression) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NReturnStatement : public NStatement {
public:
    NExpression &expression;

    NReturnStatement(NExpression &expression) : expression(expression) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NVariableDeclaration : public NStatement {
public:
    const NIdentifier &type;
    NIdentifier &id;
    NExpression *assignmentExpr;
    const bool isConst;

    NVariableDeclaration(const bool isConst, const NIdentifier &type, NIdentifier &id) : isConst(isConst), type(type),
                                                                                         id(id) { assignmentExpr = nullptr; }

    NVariableDeclaration(const bool isConst, const NIdentifier &type, NIdentifier &id, NExpression *assignmentExpr)
            : isConst(isConst), type(type), id(id), assignmentExpr(assignmentExpr) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NFunctionDeclaration : public NStatement {
public:
    const NIdentifier &type;
    const NIdentifier &id;
    VariableList arguments;
    NBlock &block;

    NFunctionDeclaration(const NIdentifier &type, const NIdentifier &id,
                         VariableList arguments, NBlock &block) : type(type), id(id), arguments(std::move(arguments)),
                                                                  block(block) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NFunctionCall : public NExpression {
public:
    const NIdentifier &id;
    ExpressionList params;

    NFunctionCall(const NIdentifier &id, ExpressionList &params) : id(id), params(params) {}

    NFunctionCall(const NIdentifier &id) : id(id) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NArrayDeclaration : public NStatement {
public:
    const NIdentifier &type;
    NIdentifier &id;
    ArrayDimension arrDim;
    ExpressionList value;

    NArrayDeclaration(NIdentifier &type, NIdentifier &id, ArrayDimension &arrDim, ExpressionList &value) : type(
            type), id(id), arrDim(arrDim), value(value) {}

    llvm::Value *codeGen(ARStack &context) override;

};

class NArrayElement : public NExpression {
public:
    const NIdentifier &id;
    ExpressionList arrayIndices;

    NArrayElement(const NIdentifier &id, ExpressionList &arrayIndices) : id(id), arrayIndices(arrayIndices) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NArrayType : public NIdentifier {
public:
    ArrayDimension arrDim;

    NArrayType(ArrayDimension &arrDim, NIdentifier &id) : arrDim(arrDim), NIdentifier(id) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NIfStatement : public NStatement {
public:
    NExpression *condition;
    NBlock *thenBlock;
    NBlock *elseBlock;

    NIfStatement(NExpression *condition, NBlock *thenBlock, NBlock *elseBlock = nullptr) :
            condition(condition), thenBlock(thenBlock), elseBlock(elseBlock) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NForStatement : public NStatement {
public:
    NStatement *init;
    NExpression *condition;
    NStatement *inc;
    NBlock *block;

    NForStatement(NStatement *init, NExpression *condition, NStatement *inc, NBlock *block) :
            init(init), condition(condition), inc(inc), block(block) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NBreakStatement : public NStatement {
public:
    llvm::Value *codeGen(ARStack &context) override;
};

class NContinueStatement : public NStatement {
public:
    llvm::Value *codeGen(ARStack &context) override;
};

class NIncOperator: public NUnaryOperator {
public:
    bool isPrefix;
    NIncOperator(int op, NExpression &rhs, bool isPrefix): NUnaryOperator(op, rhs), isPrefix(isPrefix) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NDecOperator: public NUnaryOperator {
public:
    bool isPrefix;
    NDecOperator(int op, NExpression &rhs, bool isPrefix): NUnaryOperator(op, rhs), isPrefix(isPrefix) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NWhileStatement: public NStatement {
public:
    NExpression *condition;
    NBlock *block;
    NWhileStatement(NExpression* condition, NBlock* block): condition(condition), block(block) {}

    llvm::Value *codeGen(ARStack &context) override;
};

class NDoWhileStatement: public NStatement {
public:
    NExpression *condition;
    NBlock *block;
    NDoWhileStatement(NExpression* condition, NBlock* block): condition(condition), block(block) {}

    llvm::Value *codeGen(ARStack &context) override;
};
#endif