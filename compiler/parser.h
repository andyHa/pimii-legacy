#ifndef PARSER_H
#define PARSER_H

#include <sstream>
#include <string>
#include <vector>

#include "ast.h"
#include "tokenizer.h"

struct CompilationError {
    int line;
    int pos;
    String error;
};

class Parser
{
private:
    Tokenizer& tokenizer;
    AST& ast;
    String fileName;
    std::vector<CompilationError> errors;
    std::vector< std::vector<String>* > symbolTable;

    void expect(InputTokenType tt, String rep);
    void addError(int line, int pos, String errorMsg);
    std::pair<int, int> findSymbol(String name);
    String getCurrentString();

    void block(Node* block);
    void statement(Node* block);
    Node* localAssignment();
    Node* globalAssignment();
    Node* expression();
    Node* definition();
    Node* shortDefinition();
    Node* inlineDefinition();
    void generateGuardedFunctionCode(Node* f);
    void generateFunctionCode(Node* f, bool expectBracet);
    Node* basicExp();
    Node* logExp();
    Node* relExp();
    Node* termExp();
    Node* factorExp();
    Node* inlineList();
    Node* literal();
    Atom compileLiteral();
    Node* variable();
    Node* load(String name);
    Node* call();
    Node* colonCall();
    Node* standardCall();
public:
    Parser(String fileName, Tokenizer& tokenizer, AST& ast);
    void parse();
};

#endif // PARSER_H
