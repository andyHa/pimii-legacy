#ifndef COMPILER_H
#define COMPILER_H

/**
  Language:

  $NAME -> [a..zA..Z]([a-zA-Z0-9:]*[a..zA..Z0-9])?;

  $COLON_NAME -> [a..zA..Z][a-zA-Z0-9:]*'[a..zA..Z0-9])?':';

  $BIF_NAME -> [a..zA..Z][a-zA-Z0-9:]*'[a..zA..Z0-9])?'$';

  $SYMBOL -> '#'[a..zA..Z][a-zA-Z0-9]*;

  $NUMBER -> [0-9]+;

  $STRING -> '''[^']''';

  START -> BLOCK;

  BLOCK -> STATEMENT ( ';' STATEMENT )* ';';

  STATEMENT -> ASSIGNMENT | EXPRESSION;

  ASSIGNMENT -> LOCAL_ASSIGNMENT | NORMAL_ASSIGNMENT;

  LOCAL_ASSIGNMENT -> 'var' $NAME ':=' EXPRESSION;

  NORMAL_ASSIGNMENT -> $NAME ':=' EXPRESSION;

  EXPRESSION -> DEFINITION | REL ( ( '=' | '!=' | '<' | '>' | '<=' | '>=' ) REL )?;

  DEFINITION -> NORMAL_DEFINITION | SHORT_DEFINITION | INLINE_DEFINITION;

  NORMAL_DEFINITION -> '(' $NAME ( ',' $NAME)* ')' '->' '[' BLOCK ']';

  SHORT_DEFINITION -> $NAME '->' '[' BLOCK ']';

  INLINE_DEFINITION -> '[' ( $NAME ( ',' $NAME )* '|' )? BLOCK ']';

  REL -> LOG ( ('&' | '|') LOG )*;

  LOG -> TERM ( ('+' | '-') TERM )*;

  TERM -> FACTOR ( ('*' | '/' | '%') FACTOR )*;

  FACTOR -> '(' EXPRESSION ')' | LITERAL | VARIABLE | CALL;

  LITERAL -> $NUMBER | $STRING | $SYMBOL;

  VARIABLE -> $NAME;

  CALL -> SIMPLE_CALL | COLON_CALL | BUILTIN_CALL;

  SIMPLE_CALL -> $NAME '(' ( EXPRESSION ( ',' EXPRESSION )* )? ')'

  COLON_CALL -> $COLON_NAME EXPRESSION ( $COLON_NAME EXPRESSION )*

  BUILTIN_CALL -> $BIF_NAME '(' ( EXPRESSION ( ',' EXPRESSION )* )? ')'

  */

#include <iostream>
#include <string>
#include <vector>

#include "env.h"
#include "engine.h"

class ParseException {
public:
    ParseException(int line, int pos, String error) :
        line(line),
        pos(pos),
        error(error) {}

    int line;
    int pos;
    String error;
};

enum TokenType {
    TT_EMPTY,
    TT_EOF,
    TT_NAME,
    TT_SYMBOL,
    TT_STRING,
    TT_NUMBER,
    TT_SEMICOLON,
    TT_ASSIGNMENT,
    TT_KOMMA,
    TT_DOT,
    TT_PLUS,
    TT_MINUS,
    TT_MUL,
    TT_MOD,
    TT_DIV,
    TT_AND,
    TT_OR,
    TT_EQ,
    TT_NOT,
    TT_NE,
    TT_GT,
    TT_LT,
    TT_GTEQ,
    TT_LTEQ,
    TT_L_BRACE,
    TT_R_BRACE,
    TT_L_BRACKET,
    TT_R_BRACKET,
    TT_ASM_BEGIN,
    TT_ASM_END,
    TT_ARROW
};

struct Token {
    TokenType type;
    int line;
    int pos;
    String tokenString;
    int tokenInteger;
};

class Compiler
{    
    int line;
    int pos;
    wchar_t ch;
    std::wistream& input;
    wchar_t nextChar();

    std::vector< std::vector<String>* > symbolTable;
    std::pair<int, int> findSymbol(String name);
    Token current;
    Token lookahead;
    Token lookahead2;

    Engine* engine;

    Atom file;
    Atom code;
    Atom tail;

    void fetch();
    Token fetchToken();
    void expect(TokenType tt, String rep);

    void addCode(Atom atom);

    void block();
    void statement();
    void localAssignment();
    void normalAssignment();
    void expression();
    void includeAsm();
    void handleAsmSublist();
    void definition();
    void shortDefinition();
    void inlineDefinition();
    void generateFunctionCode(bool expectBracet);
    void relExp();
    void logExp();
    void termExp();
    void factorExp();
    void literal();
    Atom compileLiteral();
    void variable();
    void load(String name);
    void call();
    void builtinCall();
    void colonCall();
    void standardCall();
public:
    Compiler(String fileName, std::wistream& inputStream, Engine* engine);

    Atom compile();
};

#endif // COMPILER_H
