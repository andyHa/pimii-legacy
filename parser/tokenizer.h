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
  Contains the tokenizer which splits a given input stream into a stream
  of tokens.
  ---------------------------------------------------------------------------
  */

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <iostream>
#include <string>
#include <vector>

#include "vm/env.h"
/**
  Enumerates the different token types.
  */
enum InputTokenType {
    // Marks a token variables a unassigned
    TT_EMPTY,
    // Marks the end of input
    TT_EOF,
    // Represents a name
    TT_NAME,
    // Represents a symbol starting with #
    TT_SYMBOL,
    // Represents a string constant
    TT_STRING,
    // Represents a numeric constant
    TT_NUMBER,
    // ;
    TT_SEMICOLON,
    // :=
    TT_ASSIGNMENT,
    // ::=
    TT_GLOBAL_ASSIGNMENT,
    // :
    TT_COLON,
    // ,
    TT_KOMMA,
    // .
    TT_DOT,
    // +
    TT_PLUS,
    // -
    TT_MINUS,
    // *
    TT_MUL,
    // %
    TT_MOD,
    // /
    TT_DIV,
    // &
    TT_AND,
    // |
    TT_OR,
    // =
    TT_EQ,
    // !
    TT_NOT,
    // !=
    TT_NE,
    // >
    TT_GT,
    // <
    TT_LT,
    // >=
    TT_GTEQ,
    // <=
    TT_LTEQ,
    // (
    TT_L_BRACE,
    // )
    TT_R_BRACE,
    // [
    TT_L_BRACKET,
    // ]
    TT_R_BRACKET,
    // {
    TT_L_CURLY,
    // }
    TT_R_CURLY,
    // <<
    TT_ASM_BEGIN,
    // >>
    TT_ASM_END,
    // ->
    TT_ARROW,
    // #(
    TT_LIST_START
};

/**
  Represents a token which is encountered during parsing.
  */
struct InputToken {
    // Contains the type of the token
    InputTokenType type;

    // Contains the position of the token
    int line;
    int pos;

    // used for syntax highlighting etc.
    int absolutePos;

    // Contains the textual representation of the token.
    String tokenString;

    // If the token type is TT_NUMNER, this cointains the
    // numeric representation of the token.
    int tokenInteger;
};

/**
  Represents the tokenizer.
  */
class Tokenizer
{

    /**
      Contains the current line
      */
    int line;
    /**
      Contains the column within the current line
      */
    int pos;

    /**
      Contains the absolute pos since teh start of the input.
      In constast: pos is relative to the current line.
      */
    int absolutePos;

    /**
      Contains the current char.
      */
    wchar_t ch;
    /**
      Contains the inputstream which provides the source code.
      */
    std::wistream& input;

    /**
      Contains the current token.
      */
    InputToken current;

    /**
      Contains the current lookahead. (Next after current).
      */
    InputToken lookahead;

    /**
      Contains the current +2 lookahead. (Next after lookahead).
      */
    InputToken lookahead2;
    /**
      Fetches the next char.
      */
    wchar_t nextChar();

    /**
      Computes the next token.
      */
    InputToken fetchToken();
public:
    /**
      Returns the current token.
      */
    InputToken getCurrent();

    /**
      Fetches the next token.
      */
    InputToken fetch();

    /**
      Returns the next token, without idscarding the current.
      */
    InputToken getLookahead();

    /**
      Returns the token after lookahead() without discarding the two
      previous.
      */
    InputToken getLookahead2();

    /**
      Initializes the tokenizer with a given stream.
      */
    Tokenizer(std::wistream& inputStream);
};

#endif // TOKENIZER_H