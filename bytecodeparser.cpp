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
  see bytecodeparser.h
 */
#include "bytecodeparser.h"
#include "tools.h"
#include <string>
#include <sstream>

BytecodeParser::BytecodeParser(std::wistream& inputStream, Engine* engine) : input(inputStream), engine(engine)
{
    token = TT_EMPTY;
    //Move to first non space character...
    input.get(ch);
    while(std::isspace(ch)) {
        input.get(ch);
    }
}



BytecodeParser::TokenType BytecodeParser::fetch()  {
    while(std::isspace(ch)) {
        input.get(ch);
    }
    token = TT_EMPTY;
    if (input.eof()) {
        token = TT_EOF;
    } else if (ch == '(') {
        tokenString = String(L"(");
        input.get(ch); // Read over character
        token = TT_L_BRACE;
    } else if (ch == ')') {
        tokenString = String(L")");
        input.get(ch); // Read over character
        token = TT_R_BRACE;
    } else if (ch == ',') {
        tokenString = String(L",");
        input.get(ch); // Read over character
        token = TT_KOMMA;
    } else if (ch == '_') {
        tokenString = String(L"NIL");
        input.get(ch); // Read over character
        token = TT_NIL;
    } else if (ch == '#') {
        tokenString = String();
        input.get(ch);
        while(std::isalnum(ch)) {
            tokenString += ch;
            input.get(ch);
        }
        token = TT_SYMBOL;
    } else if (ch == '@') {
        tokenString = String();
        input.get(ch);
        while(std::isalnum(ch)) {
            tokenString += ch;
            input.get(ch);
        }
        token = TT_GLOBAL;
    } else if (ch == '$') {
        tokenString = String();
        input.get(ch);
        while(std::isalnum(ch)) {
            tokenString += ch;
            input.get(ch);
        }
        token = TT_BIF;
    } else if (std::isdigit(ch) || ch == '-') {
        tokenString = String();
        tokenString += ch;
        input.get(ch);
        while(std::isdigit(ch)) {
            tokenString += ch;
            input.get(ch);
        }
        std::wstringstream buffer;
        buffer << tokenString;
        buffer >> tokenInteger;
        token = TT_INTEGER;
    } else if (ch == '\'') {
        tokenString = String();
        input >> ch;
        while(ch != '\'') {
            if (ch == '\\') {
                input.get(ch);
            }
            tokenString += ch;
            input.get(ch);
        }
        input.get(ch); // Read over last "
        token = TT_STRING;
    }
    if (token == TT_EMPTY) {
        throw "Unexpected Token!";
    } else {
        return token;
    }
}

Atom BytecodeParser::parse() {
   Atom result = NIL;
   Atom tail =  NIL;
    if (token == TT_EMPTY) {
            fetch();
    }
    if (token != TT_L_BRACE) {
            throw "Invalid token";
    }
    fetch();
    while (token != TT_R_BRACE) {
            if (token == TT_KOMMA && !isNil(result)) {
                    fetch();
                    Atom pair = engine->storage.makeCons(engine->storage.getCons(result)->car, parseOne());
                    if (token != TT_R_BRACE) {
                            throw "Invalid token";
                    }
                    fetch();
                    return pair;
            }
            Atom value = parseOne();
            if (isNil(tail)) {
                    result = engine->storage.makeCons(value, NIL);
                    tail = result;
            } else {
                tail = engine->storage.cons(engine->storage.getCons(tail), value);
            }
            fetch();
    }
    return result;
}



Atom BytecodeParser::parseOne() {
    if (token == TT_L_BRACE) {
        return parse();
    }
    if (token == TT_SYMBOL) {
        return  engine->storage.makeSymbol(tokenString);
    }
    if (token == TT_GLOBAL) {
        return engine->storage.findGlobal(engine->storage.makeSymbol(tokenString));
    }
    if (token == TT_BIF) {
        Atom result = engine->findBuiltInFunction(engine->storage.makeSymbol(tokenString));
        if (isNil(result)) {
            throw "Unknown BIF!";
        }
        return result;
    }
    if (token == TT_INTEGER) {
        return makeNumber(tokenInteger);
    }
    if (token == TT_STRING) {
        return engine->storage.makeString(tokenString);
    }
    if (token == TT_NIL) {
        return NIL;
    }
    throw "Unexpected";
}
