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
/**
  ---------------------------------------------------------------------------
  Contains the compiler which translates source code into bytecodes.
  ---------------------------------------------------------------------------
  */

#ifndef COMPILER_H
#define COMPILER_H

/**
  Language:

  There are several aims which influenced the design of the language:

   1) No built in keywords, so that it can be completely translated if
      desired.
   2) As few constructs as possible, so that the language is easy to learn.
   3) It should be possible to parse and compile the language in one pass.
   4) Keep everthing as simple as possible. Due to the "colon-calls" which
      are borrowed from Smalltalk, the language can be easily extended.
      Furthermore, one can always write a better / advanced compiler within
      the language itself. Therefore, this compiler won't do any magic,
      like inlining etc. The compiler and its generated code, should be
      easily understood by people new to this subject.

  The compiler is implemented as recursive descendand parser with a hand
  written tokenizer. Therefore the complete tokenizer is contained in the
  fetch() method and for each non-terminal is a method with the same name.

  Being a one pass compiler, each method directly outputs the appropriate
  bytecode for the parsed sources.

  Tokens
  -------------------------------------------------------------------

  // Represents a name of a variable or function.
  $NAME -> [a..zA..Z_]([a-zA-Z0-9_:]*[a..zA..Z0-9_])?;

  // Represents a name of a function which uses a Smalltalk like call
  // pattern: The name consists of several words which each ends with
  // a colon. Parameters are passed in after each word.
  // So the method if:then:else: is invoked, via:
  // if: [ condition ] then: [ do something ] else: [ do somethind else ]
  $COLON_NAME -> [a..zA..Z_][a-zA-Z0-9:_]*'[a..zA..Z0-9_])?':';

  // A symbol is a name for an internal constant. All op-codes are for
  // example symbols. Once translated to its internal representation,
  // symbols can be compared very fast.
  $SYMBOL -> '#'[a..zA..Z_][a-zA-Z0-9_]*;

  // Represents an integer number.
  $NUMBER -> (0|'-'?[1-9][0-9]*);

  // Represents a string constant.
  $STRING -> '''[^']''';

  Rules
  -------------------------------------------------------------------

  START -> BLOCK;

  BLOCK -> STATEMENT ( ';' STATEMENT )* ';'?;

  STATEMENT -> ASSIGNMENT | EXPRESSION;

  ASSIGNMENT -> LOCAL_ASSIGNMENT | GLOBAL_ASSIGNMENT;

  LOCAL_ASSIGNMENT -> $NAME ':=' EXPRESSION;

  GLOBAL_ASSIGNMENT -> $NAME '::=' EXPRESSION;

  EXPRESSION -> DEFINITION | BASIC;

  BASIC -> LOG ( ('&' | '|') LOG )*;

  DEFINITION -> NORMAL_DEFINITION | SHORT_DEFINITION | INLINE_DEFINITION;

  NORMAL_DEFINITION -> '(' $NAME ( ',' $NAME)* ')' '->' ('[' BLOCK ']' | STATEMENT | GUARDED);

  SHORT_DEFINITION -> $NAME '->' ('[' BLOCK ']' | STATEMENT | GUARDED);

  GUARDED -> '{' ( '[' BASIC? ':' BLOCK ']' )+ '}';

  INLINE_DEFINITION -> '[' ( $NAME ( ',' $NAME )* '->' )? BLOCK ']';

  LOG -> REL ( ( '=' | '!=' | '<' | '>' | '<=' | '>=' ) REL )?;

  REL -> TERM ( ('+' | '-') TERM )*;

  TERM -> FACTOR ( ('*' | '/' | '%') FACTOR )*;

  FACTOR -> '!' FACTOR | '(' EXPRESSION ')' | LITERAL | VARIABLE | CALL;

  LITERAL -> $NUMBER | $STRING | $SYMBOL | INLINE_LIST | ASSEMBLY;

  INLINE_LIST -> '#(' EXPRESSION ( ',' EXPRESSION )* ')';

  ASSEMBLY -> '<<' ($NUMBER | $STRING | $SYMBOL) ( ',' ($NUMBER | $STRING | $SYMBOL) )* '>>';

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
    TT_GLOBAL_ASSIGNMENT,
    TT_COLON,
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
    TT_L_CURLY,
    TT_R_CURLY,
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
    void globalAssignment();
    void expression();
    void includeAsm();
    void handleAsmSublist();
    void definition();
    void shortDefinition();
    void inlineDefinition();
    void generateFunctionCode(bool expectBracet);
    void basicExp();
    void logExp();
    void relExp();
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
