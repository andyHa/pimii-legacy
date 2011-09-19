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
  Contains the skeleton of a parser which "understands" the pimii language.
  This is used by the compiler as well as the syntax correctors and the
  HTML / TeX generator.
  ---------------------------------------------------------------------------
  */


#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <vector>

#include "vm/env.h"
#include "parser/tokenizer.h"

/**
  Represents an error which was encountered during parsing.
  */
struct ParseError {
    int line;
    int pos;
    String error;
};


class Parser;

enum DefinitionType {
    NORMAL_DEFINITION,
    SHORT_DEFINITION,
    INLINE_DEFINITION
};

class Visitor {
    std::vector<ParseError> errors;
    Parser* parser;
public:
    void setParser(Parser* parser) {
        this->parser = parser;
    }
    virtual ~Visitor(){}
    virtual void blockBegin() {}
    virtual void blockEnd() {}
    virtual void statementBegin() {}
    virtual void statementEnd() {}
    virtual void localAssignmentBegin(String name, int level, int index) {}
    virtual void localAssignmentEnd(String name, int level, int index) {}
    virtual void globalAssignmentBegin(String name, int level, int index) {}
    virtual void globalAssignmentEnd(String name, int level, int index) {}
    virtual void expressionBegin() {}
    virtual void expressionEnd() {}
    virtual void includeAsmBegin() {}
    virtual void includeAsmEnd() {}
    virtual void onAsmLiteral() {}
    virtual void asmCreateTuple(Token& left, Token& right) {}
    virtual void asmSublistBegin() {}
    virtual void asmSublistEnd() {}
    virtual void definitionBegin(DefinitionType type) {}
    virtual void definitionAddParameter(String name) {}
    virtual void definitionBodyBegin(bool brackets) {}
    virtual void definitionBodyEnd(bool brackets) {}
    virtual void definitionEnd(DefinitionType type) {}
    virtual void guardedDefinitionBegin() {}
    virtual void guardedBranchBegin() {}
    virtual void guardBegin() {}
    virtual void guardEnd() {}
    virtual void guardedBlockBegin(bool isGuarded) {}
    virtual void guardedBlockEnd() {}
    virtual void guardedDefinitionEnd() {}
    virtual void relExpBegin() {}
    virtual void relExpEnd() {}
    virtual void relExpOp() {}
    virtual void relExpContinue() {}
    virtual void basicExpBegin() {}
    virtual void basicExpEnd() {}
    virtual void basicExpOp() {}
    virtual void basicExpContinue() {}


    virtual void addError(int line, int pos, String errorMsg) {
        ParseError e;
        e.error = errorMsg;
        e.line = line;
        e.pos = pos;
        errors.push_back(e);
    }
};


class Parser
{

    /**
     Points to the tokenizer in use.
      */
    Tokenizer& tokenizer;

    /**
      Points to the visitor in use.
      */
    Visitor& visitor;

    /**
      Assumes that the current token has the given type.
      If this is true, it is consumed, otherwise an error
      is reported.
      */
    void expect(TokenType tt, String rep);

    /**
      ---------------------------------------------------------------------------
      The main parser...
      ---------------------------------------------------------------------------
      */


    /**
      Contains the current symbol table which knows all bound parameteres
      and local variables.
      */
    std::vector< std::vector<String>* > symbolTable;
    /**
      Performs a symbol lookup within the smbolTable
      */
    std::pair<int, int> findSymbol(String name);

    /**
      Parses a block of statements.
      */
    void block();

    /**
      Parses a statement.
      */
    void statement();

    /**
      Parses a local assignment.
      */
    void localAssignment();

    /**
      Parses a global assignment.
      */
    void globalAssignment();

    /**
      Parses an expression.
      */
    void expression();

    /**
      Parses inline op-codes.
      */
    void includeAsm();

    /**
      Parses inline op-codes.
      */
    void handleAsmSublist();

    /**
      Parses a function definition.
      */
    void definition();

    /**
      Parses a shortened function definition.
      */
    void shortDefinition();

    /**
      Parses an inlined function definition.
      */
    void inlineDefinition();

    /**
      Used to compile a guarded function-
      */
    void generateGuardedFunctionCode();

    /**
      Used to compile to body of a function.
      */
    void generateFunctionCode(bool expectBracet);

    /**
      Parses a basic expression.
      */
    void basicExp();

    /**
      Parses a logical expression.
      */
    void logExp();

    /**
      Parses a relational expression.
      */
    void relExp();

    /**
      Parses a term expression.
      */
    void termExp();

    /**
      Parses a factor expression.
      */
    void factorExp();

    /**
      Parses an inlined list.
      */
    void inlineList();

    /**
      Parses a literal.
      */
    void literal();

    /**
      Parses a variable.
      */
    void variable();

    /**
      Parses a function call.
      */
    void call();

    /**
      Parses a colon call.
      */
    void colonCall();

    /**
      Parses a standard call.
      */
    void standardCall();

public:
    /**
      Initializes the parser.
      */
    Parser(Tokenizer& tokenizer, Visitor& visitor);

    /**
      Invokes the parser.
      */
    void parse();

    friend class Visitor;
};

#endif // PARSER_H
