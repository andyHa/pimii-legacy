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

#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <iostream>

#include "env.h"
#include "storage.h"

class Engine;

/**
  Defines a built in function. Retrieves the stack stop,
  several parameters must be passed as list, and returns the
  result as list (may again be a list, or NIL to indicate that
  there is no result).
  */
typedef Atom (*BIF)(Engine* engine, Storage& storage, Atom param);

class Engine
{

    class Register {
        Atom value;
    public:
        Register();
        Atom read();
        void write(Atom atom);
        Atom pop();
        void push(Atom);
        String toString();
    };

    Storage storage;

    Atom s;
    Atom e;
    Atom c;
    Atom d;

    void push(Atom& reg, Atom value);

    Atom pop(Atom& reg);

    /**
      Maps symbols to unique BIF indices
      */
    LookupTable <Word, BIF, Word> bifTable;
    /**
      Registers essential built in functions.
      */
    void initializeBIF();

    void dispatch(Atom opcode);
    void dispatchArithmetic(Atom opcode);
    void dispatchBoolean(Atom opcode);

    Atom locate(Atom pos);
    void store(Atom pos, Atom value);

    void opNIL();
    void opLD();
    void opLDC();
    void opLDF();
    void opAP();
    void opRTN();
    void opSEL();
    void opJOIN();
    void opRAP();
    void opDUM();
    void opST();
    void opLDG();
    void opSTG();
    void opBAP();
    void opCAR();
    void opCDR();
    void opCONS();
    void opEQ(int a, int b);
    void opNQ(int a, int b);
    void opLE(int a, int b);
    void opGT(int a, int b);
    void opLEQ(int a, int b);
    void opGTQ(int a, int b);
    void opADD(int a, int b);
    void opSUB(int a, int b);
    void opMUL(int a, int b);
    void opDIV(int a, int b);
    void opREM(int a, int b);
    void opNOT();
    void opAND(Atom a, Atom b);
    void opOR(Atom a, Atom b);
    void opXOR(Atom a, Atom b);

    bool shouldGC();
    void gc();

    /**
      Converts the given list into a string.
      */
    String printList(Atom atom);


public:
    /**
      Registers a built in function.
      */
    Atom makeBuiltInFunction(Atom nameSymbol, BIF value);
    /**
      Looks up a bif.
      */
    Atom findBuiltInFunction(Atom nameSymbol);

    /**
      Extracts the function pointer from the given atom.
      */
    BIF getBuiltInFunction(Atom atom);

    /**
     Returns the name of the given built in function
     */
    String getBIFName(Atom atom);

    /**
      Generates a re-parseable representation of the given atom.
      */
    String toString(Atom atom);

    /**
      Generates a simple an readable representation of the given atom.
      */
    String toSimpleString(Atom atom);

    Engine();
    ~Engine();

    Atom exec(Atom code);
    Atom eval(std::wistream& stream);

    friend class BytecodeParser;
    friend class Compiler;
};

#endif // ENGINE_H
