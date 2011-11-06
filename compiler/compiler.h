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
  written tokenizer. Therefore for each non-terminal is a method with the
  same name.

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

  BLOCK -> EXPRESSION ( ';' EXPRESSION )* ';'?;

  EXPRESSION -> DEFINITION | BASIC;

  BASIC -> LOG ( ('&' | '|') LOG )*;

  DEFINITION -> NORMAL_DEFINITION | SHORT_DEFINITION | INLINE_DEFINITION;

  NORMAL_DEFINITION -> '(' $NAME ( ',' $NAME)+ ')' '->' ('[' BLOCK ']' | STATEMENT | GUARDED);

  SHORT_DEFINITION -> $NAME '->' ('[' BLOCK ']' | STATEMENT | GUARDED);

  GUARDED -> '{' ( '[' BASIC? ':' BLOCK ']' )+ '}';

  INLINE_DEFINITION -> '[' ( $NAME ( ',' $NAME )* '->' )? BLOCK ']';

  LOG -> REL ( ( '=' | '!=' | '<' | '>' | '<=' | '>=' ) REL )*;

  REL -> TERM ( ('+' | '-') TERM )*;

  TERM -> FACTOR ( ('*' | '/' | '%') FACTOR )*;

  FACTOR -> '!' FACTOR | '(' EXPRESSION ')' | LITERAL | VARIABLE | CALL | ASSIGNMENT;

  LITERAL -> $NUMBER | $STRING | $SYMBOL | INLINE_LIST;

  INLINE_LIST -> '#(' EXPRESSION ( ',' EXPRESSION )* ')';

  VARIABLE -> $NAME;

  CALL -> SIMPLE_CALL | COLON_CALL;

  SIMPLE_CALL -> $NAME '(' ( EXPRESSION ( ',' EXPRESSION )* )? ')'

  COLON_CALL -> $COLON_NAME EXPRESSION ( $COLON_NAME EXPRESSION )*

  ASSIGNMENT -> LOCAL_ASSIGNMENT | GLOBAL_ASSIGNMENT | SPLIT_ASSIGNMENT;

  LOCAL_ASSIGNMENT -> $NAME ':=' EXPRESSION;

  GLOBAL_ASSIGNMENT -> $NAME '::=' EXPRESSION;

  SPLIT_ASSIGNMENT -> $NAME | $NAME '::=' EXPRESSION;

  */

#include <QString>

#include <vector>

#include "vm/env.h"
#include "vm/engine.h"
#include "compiler/tokenizer.h"

/**
  Used to signal a compilation problem. We try to continue as long as
  possible and not to fail on the first missing ; etc.
  */
struct CompilationError {
    int line;
    int pos;
    QString error;
    bool severe;
};

/**
  Used to describe the location of a variable in the environment.
  */
class EnvPos {
public:
    int major;
    int minor;

    EnvPos(int major, int minor) : major(major), minor(minor) {}
};

/**
  Contains the main part of the compiler. An instance is used to compile one
  source file and should not be reused.
  */
class Compiler
{
private:

    /**
      Contains the last linenumer which was added to the code. This is used,
      to generate accurate position information within the code.
      */
    int lastLine;

    /**
      Contains the engine for which the code is compiled.
      */
    Engine* engine;

    /**
      Contains the tokenizer, which transforms the code.
      */
    Tokenizer tokenizer;

    /**
      Contains the symbol which represents the file name.
      */
    AtomRef* file;

    /**
      Contains the start of the generated op-codes
      */
    AtomRef* code;

    /**
      Points to the last cell of th generated op codes.
      */
    AtomRef* tail;

    /**
      Contains a list of all discovered errors.
      */
    std::vector<CompilationError> errors;

    /**
      Contains the current symbol table (all known variable names). The
      indices within these nested vectors directly reflects the position
      within the environment of the execution engine.
      */
    std::vector< std::vector<QString>* > symbolTable;

    /**
      Checks if the position in the source file changed and generates
      appropriate op-codes to notify the interpreter.
      */
    void updatePosition(bool force);

    /**
      Used to lookup a symbol in the symbol table. If nothing is found, the
      major index is negative.
      */
    EnvPos findSymbol(QString name);

    /**
      Adds an op code or value to the resulting code-list.
      */
    void addCode(Atom atom);

    /**
      Expects the given token as next in the tokenizer, or created an error
      otherwise.
      */
    void expect(InputTokenType tt, const char* rep);

    /**
      Compiles a block which is a list of statements.
      */
    void block();

    /**
      Compiles a statement.
      */
    void statement();

    /**
      Compiles an expression.
      */
    void expression();

    /**
      Compiles a function definition.
      */
    void definition();

    /**
      Compiles a shortened function definition: x -> ...
      */
    void shortDefinition();

    /**
      Compiles an inline function definition: [ x -> ...] or [...]
      */
    void inlineDefinition();

    /**
      Compiles a guarded function: {[x > 3 : ...]}
      */
    void generateGuardedFunctionCode();

    /**
      Compiles a function body.
      */
    void generateFunctionCode(bool expectBracet, bool asSublist);

    /**
      Compiles: && and ||
      */
    void basicExp();

    /**
      Compiles: + - &
      */
    void logExp();

    /**
      Compiles: < <= = >= > !=
      */
    void relExp();

    /**
      Compiles: * / %
      */
    void termExp();

    /**
      Compiles ! or expression in braces.
      */
    void factorExp();

    /**
      Compiles a literal.
      */
    void literal();

    /**
      Compiles an inline list.
      */
    void inlineList();

    /**
      Compiles inlined XML
      */
    void inlineXML();

    /**
      Compiles an XML tag.
      */
    void handleTag();

    /**
      Compiles a self closing tag: <xml />
      */
    void handleTagSelfClose(QString& tag);

    /**
      Compiles the end of a tag definition.
      */
    void handleTagEnd(QString& tag, QString& tagName);

    /**
      Compiles an end tag when no body was found.
      */
    void handleTagEndEmpty(QString& tag, QString& tagName);

    /**
      Compiles an end tag when a body was found.
      */
    void handleTagEndFilled(QString& tag, QString& tagName);

    /**
      Compiles the tag content.
      */
    void handleTagContent();

    /**
      Compiles a parameter in a tag definition.
      */
    void handleTagParameter(QString& tag);

    /**
      Handles a direct parameter value.
      */
    void handleTagParameterPlain(QString& param, QString& tag);

    /**
      Compiles an interpreted parameter value.
      */
    void handleTagParameterInterpreted(QString& param, QString& tag);

    /**
     Transforms a literal into an atom.
     */
    Atom compileLiteral();

    /**
      Compiles a variable reference.
      */
    void variable();

    /**
      Generates a load operation for the given name.
      */
    void load(QString name);

    /**
      Compiles a function call.
      */
    void call();

    /**
      Compiles a function call with colons, like: if: x then: y
      */
    void colonCall();

    /**
      Compiles a standard call, like f(x)
      */
    void standardCall();

    /**
      Compiles a lokal assignment: x := 1;
      */
    void localAssignment();

    /**
      Compiles a split assignment like: x|y := a; This is the same as:
      if (isCons(a) {
         x := car(a);
         y := car(cdr(a));
         push TRUE
      } else {
         push FALSE
      }
     */
    void splitAssignment();

    /**
      Compiles a global assignment like x ::= 1;
      */
    void globalAssignment();

    /**
      Adds an error message.
      */
    void addError(const InputToken& token, const QString& errorMsg);
public:

    /**
      Creates a new compiler for the given filename, with the given content,
      for compilation within the given engine.
      */
    Compiler(const QString& fileName, const QString& input, Engine* engine);

    ~Compiler() {
        delete code;
        delete tail;
        delete file;
    }

    /**
      Invokes the compilation. Returns true, if the compilation was successfull,
      false otherwise.
      */
    bool compile(bool appendStop);

    /**
      Provides access to the generated code.
      */
    Atom getCode();

    /**
      Provides access to the generated error list.
      */
    std::vector<CompilationError> getErrors();
};

#endif // COMPILER_H
