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

#include "vm/env.h"
#include "vm/storage.h"
#include "tools/logger.h"

#include <deque>

#include <QObject>
#include <QString>
#include <QElapsedTimer>
#include <QSettings>
#include <QDir>

/**
   Forward reference - see: callcontext.h
   */
class CallContext;

/**
  Defines a built in function. Parameters and the result are passed via the
  given CallContext.
  */
typedef void (*BIF)(const CallContext& ctx);

/**
  The engine keeps an internal stack of executions. Each call to eval, pushes
  another execution on the stack. The engine runs as long as there are
  executions available.
  */
struct Execution {
    QString filename;
    AtomRef* fn;
};

/**
  An Engine operates on the given Storage and performs the actual exection of
  the bytecodes. It is basically a SECD machine with one additional register
  which represents the call-stack for better error messages (stack traces).
  */
class Engine : public QObject
{
private:

    Q_OBJECT

    /**
      The logger used by the engine.
      */
    Logger log;

    /**
      Used by "panic" to stop the engine.
      */
    class PanicException : public std::exception {};

    /**
      Provides access to the system settings.
      */
    QSettings* settings;

    /**
      Dirty hack! Contains the message of the last call to panic. Is handled
      within the interpert function!
      */
    QString lastError;

    /**
      Determines if the debug output of the compiler is turned on.
      */
    bool debugCompiler;

    /**
      Represents the storage one which the engine operates.
      */
    Storage storage;

    /**
      Used to interrupt the current execution.
      */
    volatile bool running;

    /**
      Keeps items to be executed.
      */
    std::deque<Execution> executionStack;

    /**
      Conts the total instructions executed.
      */
    Word instructionCounter;

    /**
      Contains the max number of op codes executed in the interpret function
      which is called in the event loop.
      */
    Word opCodesInInterpret;

    /**
      Represents the stack register on which most of the computations are
      performed.
      */
    AtomRef* const s;

    /**
      Represents the environment register. Contains the bound environment
      which are parameters and visible local variables.
      */
    AtomRef* const e;

    /**
      Represents the code register, which contains the bytecode of the
      current function.
      */
    AtomRef* const c;

    /**
      Represents the dump register, which is used to buffer the other
      registers during invocation of other functions etc.
      */
    AtomRef* const d;

    /**
      Represents the position register. Contains a pair (file.line) for each
      function application.
      */
    AtomRef* const p;

    /**
      Returns the nth item of the given list or register.
      */
    Atom nth(Atom list, Word idx);

    /**
      Returns the head of the given list or register.
      */
    Atom head(AtomRef* list);

    /**
      Pushes a value on the given machine register.
      */
    inline void push(AtomRef* reg, Atom value);

    /**
      Pops a value from the given machine register.
      */
    Atom pop(AtomRef* reg);

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
      Contains the path to the pimii installation.
      */
    QDir homeDir;

    /**
      Registers essential built in functions.
      */
    void initializeBIF();

    /**
      Invokes the appropriate method for the given bytecode.
      */
    inline void dispatch(Atom opcode);

    /**
      * Converts two numeric atoms into double values.
      */
    void convertNumeric(Word atoma, Word atomb, double* a, double* b);

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
    inline void opNIL();

    /**
      Loads a given location on the stack.
      */
    inline void opLD();

    /**
      Stores the stack top on a location.
      */
    inline void opST();

    /**
      Loads a constant onto the stack.
      */
    inline void opLDC();

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
    inline void opBT();

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

    /**
      Implements the equals operation for numbers, strings, symbols
      and cons-cells. This does not work with complete lists (yet)!
      */
    void opEQ();

    /**
      Opposite of EQ
      */
    void opNE();

    /**
      Compares two given elements with natural order
      (everything except lists)
      */
    void opLT();
    void opGT();
    void opLTQ();
    void opGTQ();

    /**
      Concatenates lists or strings. Therefore at least on argument
      must bei either a string or a list. If both, a list and a string
      is given, a list concatenation is performed.
      */
    void opCONCAT();

    /**
      Pops two arguments of the stack. Converts them to a string if neccessary,
      concatenates them and pushes the result on the stack. This is heavily
      used by the compiler when translating inline XML.
      */
    void opSCAT();

    /**
      Perform the given boolean logic operations.
      */
    void opNOT();
    void opAND();
    void opOR();

    /**
      Sets the file info of the interpreter.
      */
    void opFile();

    /**
      Sets the line info of the interpreter.
      */
    void opLine();

    /**
      Converts the given list into a string.
      */
    QString printList(Atom atom);

    /**
      Compiles the given stream and handles all errors.
      */
    Atom compileStream(const QString& file,
                       const QString& input,
                       bool insertStop);

    /**
      Loads the next execution in the engine and returns TRUE. If no execution
      is available, FALSE is returned.
      */
    bool loadNextExecution();

    Q_DISABLE_COPY(Engine)

signals:
    /**
      Emitted if the engine hit a hard error and cannot resume.
      */
    void onEnginePanic(Atom file,
                       Word line,
                       const QString& error,
                       const QString& status);

    /**
      Emitted if the engine sends a message.
      */
    void onLog(const QString& message);

    /**
      Emitted if the engine enters a new line, can be used by a debugger.
      */
    void onLine(Atom file, Word line);

    /**
      Emitted if an execution is completely processed.
      */
    void onExecutionFinished();

    /**
      Emitted if the engine started because new work arrived.
      */
    void onEngineStarted();

    /**
      Emitted if the engine has no more work to do.
      */
    void onEngineStopped();

public slots:
    /**
      Compiles and loads the given source. Will be pushed on the
      "executionStack".
      */
    void eval(const QString& source, const QString& filename);

public:
    /**
      Generates an engine-panic if the given expectation is not true.
      This is almost like "assert" but will output more information in case
      of an error.
      */
    inline void expect(bool expectation,
                const char* errorMessage,
                const char* file,
                int line) {
        if (!expectation) {
            panic(errorMessage +
                  QString(" (") +
                  QString(file) +
                  QString(":") +
                  intToString(line) +
                  QString(")"));
        }
    }

    /**
      Stops the execution, writes the contents of all registers and a
      stacktrace to the standard output.
      */
    void panic(const QString& error);

    /**
      Provides direct access to the settings object.
      */
     QSettings* getSettings();

    /**
      Prints details of the engine status.
      */
    QString stackDump();

    /**
      Finds an absolute path for the given fileName by checking all source
      directories.
      */
    QString lookupSource(const QString& fileName);

    /**
      Returns the selected home-path of the engine.
      */
    QDir home() {
        return homeDir;
    }

    /**
      Compiles the given file and returns the given bytecode. If an error
      occures during compilation an appropriate message is shown and the
      engine is stopped.
      */
    Atom compileFile(const QString& file, bool insertStop);

    /**
      Compiles the given source code and returns the given bytecode. If an
      error occures during compilation an appropriate message is shown and
      the engine is stopped (unless slient is true, then nothing happens and
      NIL is returned).
      */
    Atom compileSource(const QString& filename,
                       const QString& source,
                       bool insertStop,
                       bool silent);

    /**
      Makes the interpreter jump into the given list for execution. This
      is more or less like a call without parameters.
      */
    void call(Atom list);

    /**
      Prints the given string.
      */
    void println(const QString& string);

    /**
      Registers a built in function.
      */
    Atom makeBuiltInFunction(Atom nameSymbol, BIF value);

    /**
      Registers a built in function.
      */
    Atom makeBuiltInFunction(const char* name, BIF value);

    /**
      Looks up a bif.
      */
    Atom findBuiltInFunction(Atom nameSymbol);

    /**
      Looks up a bif for a given char*
      */
    Atom findBuiltInFunction(const char* name);

    /**
      Extracts the function pointer from the given atom.
      */
    BIF getBuiltInFunction(Atom atom);

    /**
     Returns the name of the given built in function
     */
    QString getBIFName(Atom atom);

    /**
      Generates a re-parseable representation of the given atom.
      */
    QString toString(Atom atom);

    /**
      Generates a simple an readable representation of the given atom.
      */
    QString toSimpleString(Atom atom);

    /**
      Continues the current evaluation.
      */
    void interpret();

    /**
      Returns if the engine has executable work to run.
      */
    bool isRunnable();

    /**
      Flushes the execution stack and stops the engine.
      */
    void terminate();

    /**
      Creates a new engine.
      */
    Engine(QSettings* settings);

    /**
      Tries to find the pimii home installation.
      */
    void initializeSourceLookup();

    /**
      Initializes the engine.
      */
    void initialize();

    ~Engine();

    /**
      Used by the management interface (core extension provides an
      engine::setValue function to set the internal state of the engine.
      This provides an extensible mechanism to change various internal
      variables. The given name must be a symbol. The given value will be
      checked by the function.
      */
    void setValue(Atom name, Atom value);

    /**
      Reads an internal variable. See setValue for more information.
      */
    Atom getValue(Atom name);

    friend class Compiler;
};

#endif // ENGINE_H
