/**
    Licensed to the Apache Software Foundation (ASF) under one
    or more contributor license agreements.  See the NOTICE file
    distributed with this work for additional information
    regarding copyright ownership.  The ASF licenses this file
    to you under the Apache License, Version 2.0 (the
    "License"); you may not use this file except in compliance
    with the License.  You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing,
    software distributed under the License is distributed on an
    "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
    KIND, either express or implied.  See the License for the
    specific language governing permissions and limitations
    under the License.
 */
// ---------------------------------------------------------------------------
// Contains the compiler which translates source code into bytecodes.
// ---------------------------------------------------------------------------

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

  INLINE_DEFINITION -> '[' ( $NAME ( ',' $NAME )* '->' )? BLOCK ']';

  REL -> LOG ( ('&' | '|') LOG )*;

  LOG -> TERM ( ('+' | '-') TERM )*;

  TERM -> FACTOR ( ('*' | '/' | '%') FACTOR )*;

  FACTOR -> '(' EXPRESSION ')' | LITERAL | VARIABLE | CALL;

  LITERAL -> $NUMBER | $STRING | $SYMBOL | INLINE_LIST;

  INLINE_LIST -> '#(' EXPRESSION ( ',' EXPRESSION )* ')';

  VARIABLE -> $NAME;

  CALL -> SIMPLE_CALL | COLON_CALL;

  SIMPLE_CALL -> $NAME '(' ( EXPRESSION ( ',' EXPRESSION )* )? ')'

  COLON_CALL -> $COLON_NAME EXPRESSION ( $COLON_NAME EXPRESSION )*

  */

#include <iostream>
#include <string>
#include <vector>

#include "env.h"
#include "engine.h"

struct CompilationError {
    int line;
    int pos;
    String error;
    bool severe;
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
    TT_ARROW,
    TT_LIST_START
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

    std::vector<CompilationError> errors;

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
    void inlineList();
    void literal();
    Atom compileLiteral();
    void variable();
    void load(String name);
    void call();
    void colonCall();
    void standardCall();

    void addError(int line, int pos, String errorMsg);
public:
    Compiler(String fileName, std::wistream& inputStream, Engine* engine);

    std::pair< Atom, std::vector<CompilationError> > compile(bool appendStop);
};

#endif // COMPILER_H
