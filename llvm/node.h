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
typedef std::vector<uint32_t> ArrayDimension;

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
    uint32_t length;
    std::vector<std::string> initList;

    NArray(NIdentifier type, const std::vector<std::string> &initList) : type(std::move(type)), initList(initList) {
        length = initList.size();
    }

    NArray(NIdentifier type, int32_t length) : type(std::move(type)), length(length) {}

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
    NInteger &index;

    NArrayAssignment(NIdentifier &lhs, NInteger &index, NExpression &rhs)
            : index(index), NAssignment(lhs, rhs) {}

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
    NExpression &expression;

    NExpressionStatement(NExpression &expression) : expression(expression) {}

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
                         VariableList arguments, NBlock &block) :
            type(type), id(id), arguments(std::move(arguments)), block(block) {}

    llvm::Value *codeGen(ARStack &context) override;
};

#endif