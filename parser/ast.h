#ifndef AST_H
#define AST_H

#include <vector>

class Node
{
public:
    int startPos;
    int startLine;
    int endPos;
    int endLine;
};

class Statement : public Node {

};

class Assignment : public Statement {
public:
    String var;
    bool global;
    Expression expr;
};

class Block : public Node {
public:
    std::vector<Statement*> statements;
};

class Expression : public Statement {

};

class BinaryOperation : public Expression {
public:
    String op;
    Word   opSymbol;
    Expression left;
    Expression right;
};

class Braced : public Expression {
public:
    Expression inner;
};

class Definition : public Expression {
public:
    std::vector<String> formalParameters;
    bool braced;
    bool inlined;
};

class SimpleDefinition : public Definition {
public:
    Block body;
};

class GuardedBlock {
public:
    bool hasGuard;
    Expression guard;
    Block body;
};

class GuardedDefinition : public Definition {
public:
    std::vector<GuardedBlock> blocks;
};


class Call : public Expression {

};


#endif // AST_H
