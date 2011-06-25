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
#ifndef BYTECODEPARSER_H
#define BYTECODEPARSER_H

#include <iostream>
#include "env.h"
#include "engine.h"

class BytecodeParser
{
    enum TokenType {
        TT_EMPTY,
        TT_EOF,
        TT_L_BRACE,
        TT_R_BRACE,
        TT_SYMBOL,
        TT_GLOBAL,
        TT_BIF,
        TT_INTEGER,
        TT_KOMMA,
        TT_STRING,
        TT_NIL
    };

    TokenType token;
    String tokenString;
    int tokenInteger;
    wchar_t ch;
    std::wistream& input;
    TokenType fetch();
    Atom parseOne();
    Engine* engine;
public:
   BytecodeParser(std::wistream& inputStream, Engine* engine);

   Atom parse();
};

#endif // BYTECODEPARSER_H
