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
  Contains the evaluation engine which performs the computations.
  ---------------------------------------------------------------------------
  */

#ifndef ENGINE_H
#define ENGINE_H

#include "env.h"
#include "storage.h"
#include "interceptor.h"

#include <QString>
#include <QElapsedTimer>

/**
  Forward reference for the declaration of built in functions.
  */
class Engine;

/**
  Defines a built in function. Retrieves the stack stop,
  several parameters must be passed as list, and returns the
  result as list (may again be a list, or NIL to indicate that
  there is no result).
  */
typedef Atom (*BIF)(Engine* engine, Storage& storage, Atom param);

/**
  An Engine operates on the given Storage and performs the actual exection of
  the bytecodes. It is basically a SECD machine with one additional register
  which represents the call-stack for better error messages (stack traces).
  */
class Engine
{
    /**
      Used by "panic" to stop the engine.
      */
    class StopEngineException : public std::exception {};

    /**
      Represents the storage one which the engine operates.
      */
    Storage storage;

    /**
      Used to log output.
      */
    Interceptor* interceptor;

    /**
      Used to interrupt the current execution.
      */
    volatile bool running;

    /**
      Conts the total instructions executed.
      */
    Word instructionCounter;

    /**
      Counts the total number of garbage collections.
      */
    Word gcRuns;

    /**
      Tracks execution duration.
      */
    QElapsedTimer timer;

    /**
      Contains the state of timer since the last status report.
      */
    Word lastStatusReport;

    /**
      Contains the state of timer sind the last GC.
      */
    Word lastGC;

    /**
      Min number of cells allocated, before GC becomes active.
      */
    static const Word MIN_HEAP_SIZE = 4096;

    /**
      If the heap grows above this limit, we perform GCs in ver short
      intervals. This is the "heavy duty" mode.
      */
    static const Word MIN_HEAVY_GC_SIZE = 4096 * 4096;

    /**
      Determines the minimal interval as number of instructions before a
      new GC is executed in heavy duty mode.
      */
    static const Word GC_MIN_WAIT = 100;

    /**
      Determines the interval as number of instructions before a new GC
      is executed in normal mode (heap is between 50% and 75% full).
      */
    static const Word GC_WAIT = 10000;

    /**
      Determines the interval of milliseconds in which status updates
      are generted.
      */
    static const Word REPORT_INTERVAL = 100000;

    /**
      Represents the stack register on which most of the computations are
      performed.
      */
    Atom s;
    /**
      Represents the environment register. Contains the bound environment
      which are parameters and visible local variables.
      */
    Atom e;
    /**
      Represents the code register, which contains the bytecode of the
      current function.
      */
    Atom c;
    /**
      Represents the dump register, which is used to buffer the other
      registers during invocation of other functions etc.
      */
    Atom d;
    /**
      Represents the position register. Contains a pair (file.line) for each
      function application.
      */
    Atom p;

    /**
      Returns the nth item of the given list or register.
      */
    Atom nth(Atom list, Word idx);

    /**
      Returns the head of the given list or register.
      */
    Atom head(Atom list);

    /**
      Pushes a value on the given machine register.
      */
    void push(Atom& reg, Atom value);

    /**
      Pops a value from the given machine register.
      */
    Atom pop(Atom& reg);

    /**
      Contains the currently active file. This is set by the #FILE bytecode.
      */
    Atom currentFile;

    /**
      Contains the currently active line. This is set by the #LINE bytecode.
      */
    Word currentLine;

    /**
      Maps symbols to unique BIF indices
      */
    LookupTable <Word, BIF, Word> bifTable;

    /**
      Contains a list of paths which will be used to search source files.
      */
    std::vector<String> sourcePaths;

    /**
      Registers essential built in functions.
      */
    void initializeBIF();

    /**
      Invokes the appropriate method for the given bytecode.
      */
    void dispatch(Atom opcode);

    /**
      Invokes the appropriate arithmetic method for the given bytecode.
      */
    void dispatchArithmetic(Atom opcode);

    /**
      Used to lookup a location on the environment stack.
      */
    Atom locate(Atom pos);

    /**
      Used to wrtie a value on the environment stack.
      */
    void store(Atom pos, Atom value);

    /**
      Pushes NIL onto the stack.
      */
    void opNIL();

    /**
      Loads a given location on the stack.
      */
    void opLD();

    /**
      Stores the stack top on a location.
      */
    void opST();

    /**
      Loads a constant onto the stack.
      */
    void opLDC();

    /**
      Generates a closure.
      */
    void opLDF();

    /**
      Invokes a function.
      */
    void opAP(bool hasArguments);

    /**
      Returns from a function.
      */
    void opRTN();

    /**
      Branches is the stack top is true.
      */
    void opBT();

    /**
      Loads a global onto the stack.
      */
    void opLDG();

    /**
      Stores the stack top in a global.
      */
    void opSTG();

    /**
      Returns the CAR field of the stack top.
      */
    void opCAR();

    /**
      Returns the CDR field of the stack top.
      */
    void opCDR();

    /**
      Generates a new cell.
      */
    void opCONS();

    /**
      Splits a given cons cell.
      */
    void opSPLIT();

    /**
      Pops a list and a value from the stack,
      and appends the value to the list.
      */
    void opCHAIN();

    /**
      Pushes the final list onto the stack which was
      created by a sequence of CHAIN bytecodes.
      */
    void opCHAINEND();

    void opEQ();
    void opNE();
    void opLT();
    void opGT();
    void opLTQ();
    void opGTQ();
    void opADD();
    void opSUB(int a, int b);
    void opMUL(int a, int b);
    void opDIV(int a, int b);
    void opREM(int a, int b);
    void opNOT();
    void opAND();
    void opOR();
    void opFile();
    void opLine();

    bool shouldGC();
    void gc();

    /**
      Converts the given list into a string.
      */
    String printList(Atom atom);

    /**
      Compiles the given stream and handles all errors.
      */
    Atom compileStream(const QString& file, const QString& input, bool insertStop);


public:
    /**
      Generates an engine-panic if the given expectation is not true.
      This is almost like "assert" but will output more information in case
      of an error.
      */
    void expect(bool expectation, const char* errorMessage, const char* file, int line);

    /**
      Stops the execution, writes the contents of all registers and a
      stacktrace to the standard output.
      */
    void panic(String error);

    /**
      Adds the given directory as source lookup path.
      */
    void addSourcePath(String path);

    /**
      Finds an absolute path for the given fileName by checking all source
      directories.
      */
    String lookupSource(String fileName);

    /**
      Compiles the given file and returns the given bytecode. If an error
      occures during compilation an appropriate message is shown and the
      engine is stopped.
      */
    Atom compileFile(String file, bool insertStop);

    /**
      Compiles the given source code and returns the given bytecode. If an error
      occures during compilation an appropriate message is shown and the
      engine is stopped.
      */
    Atom compileSource(String filename, String source, bool insertStop);

    /**
      Prints the given string.
      */
    void println(String string);

    /**
      Reports the engine status to the current interceptor.
      */
    void reportStatus();

    /**
      Registers a built in function.
      */
    Atom makeBuiltInFunction(Atom nameSymbol, BIF value);

    /**
      Registers a built in function.
      */
    Atom makeBuiltInFunction(String name, BIF value);

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

    /**
      Compiles and loads the given source. With the next call to continue,
      the code will be evaluated.
      */
    void prepareEval(String source, String filename);

    /**
      Interrupts the current execution.
      */
    void interrupt();

    /**
      Continues the current evaluation.
      */
    void continueEvaluation();

    Engine(Interceptor* interceptor);
    ~Engine();

    friend class Compiler;
};

#endif // ENGINE_H
