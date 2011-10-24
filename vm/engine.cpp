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

#include "engine.h"
#include "bif/callcontext.h"
#include "bif/engineextension.h"
#include "bif/coreextension.h"
#include "bif/filesextension.h"
#include "bif/webextension.h"
#include "compiler/compiler.h"

#include <QElapsedTimer>
#include <QFile>
#include <QPluginLoader>

#include <sstream>
#include <exception>

void Engine::push(AtomRef* reg, Atom atom) {
   reg->atom(storage.makeCons(atom, reg->atom()));
}

Atom Engine::pop(AtomRef* reg) {
    if (!isCons(reg->atom())) {
        return NIL;
    }
    Cons cons = storage.getCons(reg->atom());
    Atom result = cons->car;
    reg->atom(cons->cdr);
    return result;
}

Engine::Engine(QSettings* settings) :
    settings(settings),
    s(storage.ref(NIL)),
    e(storage.ref(NIL)),
    c(storage.ref(NIL)),
    d(storage.ref(NIL)),
    p(storage.ref(NIL))
{
    running = false;
    opCodesInInterpret = settings->value(
                toSimpleString(SYMBOL_VALUE_OP_CODES_PER_EVENT_LOOP),
                1000).toUInt();
    initializeSourceLookup();
}

void Engine::initializeSourceLookup() {
    QString homePath = settings->value(toSimpleString(SYMBOL_VALUE_HOME_PATH),
                                       "").toString();
    if (!homePath.isEmpty()) {
        homeDir = QDir(homePath);
        if (homeDir.exists()) {
            return;
        }
    }
    homeDir = QDir(QFileInfo(QDir::home(),
                             QString("pimii")).absoluteFilePath());
    if (homeDir.exists()) {
        return;
    } else {
        homeDir = QDir();
    }
}

Engine::~Engine() {
    delete s;
    delete e;
    delete c;
    delete d;
    delete p;
}

void Engine::initialize() {
    this->initializeBIF();
}

Atom Engine::makeBuiltInFunction(Atom nameSymbol, BIF value) {
    expect(isSymbol(nameSymbol),
           "nameSymbol is not a symbol",
           __FILE__,
           __LINE__);
    Word result = bifTable.add(nameSymbol, value);
    assert(result < MAX_INDEX_SIZE);
    return tagIndex(result, TAG_TYPE_BIF);
}

Atom Engine::makeBuiltInFunction(const char *name, BIF value) {
   return makeBuiltInFunction(storage.makeSymbol(QString(name)), value);
}

Atom Engine::findBuiltInFunction(Atom nameSymbol) {
    expect(isSymbol(nameSymbol),
           "nameSymbol is not a symbol",
           __FILE__,
           __LINE__);
    Word index = 0;
    if (!bifTable.find(nameSymbol, &index)) {
        return NIL;
    }
    return tagIndex(index, TAG_TYPE_BIF);
}

Atom Engine::findBuiltInFunction(const char* name) {
    Atom nameSymbol = storage.makeSymbol(QString(name));
    Word index = 0;
    if (!bifTable.find(nameSymbol, &index)) {
        return NIL;
    }
    return tagIndex(index, TAG_TYPE_BIF);
}

BIF Engine::getBuiltInFunction(Atom atom) {
    expect(isBIF(atom),
           "given atom is not a built in function",
           __FILE__,
           __LINE__);
    return bifTable.getValue(untagIndex(atom));
}

QString Engine::getBIFName(Atom atom) {
    expect(isBIF(atom),
           "given atom is not a built in function",
           __FILE__,
           __LINE__);
    return storage.getSymbolName(bifTable.getKey(untagIndex(atom)));
}

void Engine::gc() {
    storage.gc();
}

void Engine::opNIL() {
    push(s, NIL);
}

void Engine::opLD() {
    push(s, locate(pop(c)));
}

void Engine::opLDC() {
    push(s, pop(c));
}

void Engine::opST() {
   Atom val = pop(s);
   store(pop(c), val);
   push(s, val);
}

void Engine::opLDG() {
    Atom gobal = pop(c);
    expect(isGlobal(gobal),
           "#LDG: code top was not a global",
           __FILE__,
           __LINE__);
    push(s, storage.readGlobal(gobal));
}

void Engine::opSTG() {
    Atom gobal = pop(c);
    Atom val = pop(s);
    expect(isGlobal(gobal),
           "#STG: code top was not a global",
           __FILE__,
           __LINE__);
    storage.writeGlobal(gobal, val);
    push(s, val);
}

void Engine::opBT() {
    Atom discriminator = pop(s);
    Atom ct = pop(c);
    if (discriminator == SYMBOL_TRUE) {
        c->atom(ct);
    }
}

void Engine::opLDF() {
    push(s, storage.makeCons(pop(c), e->atom()));
}

Atom Engine::nth(Atom list, Word idx) {
    while(isCons(list) && idx > 0) {
        idx--;
        list = storage.getCons(list)->cdr;
    }
    if (isCons(list)) {
        return storage.getCons(list)->car;
    } else {
        return NIL;
    }
}

Atom Engine::head(AtomRef* list) {
    if (isCons(list->atom())) {
       return storage.getCons(list->atom())->car;
    } else {
        return NIL;
    }
}

void Engine::opAP(bool hasArguments) {
    Atom name = pop(c);
    Atom fun = pop(s);
    Atom v = NIL;
    if (hasArguments) {
        v = pop(s);
    }
    if (isBIF(fun)) {
        BIF bif = getBuiltInFunction(fun);
        CallContext ctx(this, &storage, v);
        bif(ctx);
        push(s, ctx.getResult());
    } else {
        if (!isCons(fun)) {
         panic(
           QString("'%1' is neither a closure nor a built in function (%2:%3)")
                        .arg(storage.getSymbolName(name),
                             QString(__FILE__),
                             intToString(__LINE__)));
        }
        Cons funPair = storage.getCons(fun);
        if ((head(c) == SYMBOL_OP_RTN) && (funPair->car == head(d))) {
            // We have a tail recursion -> don't push useless stuff on the
            // dump-stack, only flush stack, restart code and environment
            // (with new args)
            s->atom(NIL);
            c->atom(funPair->car);
            e->atom(storage.makeCons(v, funPair->cdr));
        } else {
            push(d, e->atom());
            push(d, s->atom());
            push(d, c->atom());
            s->atom(NIL);
            c->atom(funPair->car);
            push(d, c->atom());
            e->atom(storage.makeCons(v, funPair->cdr));
            push(p, storage.makeCons(currentFile,
                                     storage.makeNumber(currentLine)));
        }
    }
}

void Engine::opRTN() {
    Atom result = pop(s);
    pop(d);
    c->atom(pop(d));
    s->atom(pop(d));
    push(s, result);
    e->atom(pop(d));
    pop(p);
}

void Engine::opCAR() {
    Atom atom = pop(s);
    expect(isCons(atom),
           "#CAR: stack top was not a cons!",
           __FILE__,
           __LINE__);
    Cons cons = storage.getCons(atom);
    push(s, cons->car);
}

void Engine::opCDR() {
    Atom atom = pop(s);
    expect(isCons(atom),
           "#CDR: stack top was not a cons!",
           __FILE__,
           __LINE__);
    Cons cons = storage.getCons(atom);
    push(s, cons->cdr);
}

void Engine::opCONS() {
    Atom b = pop(s);
    Atom a = pop(s);
    push(s, storage.makeCons(a,b));
}

void Engine::opSPLIT() {
    Atom element = pop(s);
    Atom l1 = pop(c);
    Atom l2 = pop(c);
    if (isCons(element)) {
        Cons c = storage.getCons(element);
        if (isGlobal(l1)) {
            storage.writeGlobal(l1, c->car);
        } else if (isCons(l1)) {
            store(l1, c->car);
        }
        if (isGlobal(l2)) {
            storage.writeGlobal(l2, c->cdr);
        } else if (isCons(l2)) {
            store(l2, c->cdr);
        }
        push(s, SYMBOL_TRUE);
    } else {
        push(s, SYMBOL_FALSE);
    }
}

void Engine::opCHAIN() {
    Atom element = pop(s);
    Atom cell = pop(s);
    if (isNil(cell)) {
        Atom a = storage.makeCons(element, NIL);
        push(s, storage.makeCons(a, a));
    } else {
        expect(isCons(cell),
               "#CHAIN: stack top was not a cons!",
               __FILE__,
               __LINE__);
        Cons c = storage.getCons(cell);
        Cons tail = storage.getCons(c->cdr);
        tail->cdr = storage.makeCons(element, NIL);
        c->cdr = tail->cdr;
        push(s, cell);
    }
}

void Engine::opCHAINEND() {
    Atom cell = pop(s);
    if (!isCons(cell)) {
        push(s, storage.makeCons(cell, NIL));
    } else {
        Cons c = storage.getCons(cell);
        push(s, c->car);
    }
}

void Engine::opEQ() {
    Atom b = pop(s);
    Atom a = pop(s);
    if (a == b) {
        push(s,SYMBOL_TRUE);
    } else if (isString(a) && isString(b)) {
        push(s,storage.getString(a) ==
             storage.getString(b) ? SYMBOL_TRUE : SYMBOL_FALSE);
    } else {
        push(s,SYMBOL_FALSE);
    }
}

void Engine::opNE() {
    Atom b = pop(s);
    Atom a = pop(s);
    if (a == b) {
        push(s,SYMBOL_FALSE);
    } else if (isString(a) && isString(b)) {
        push(s,storage.getString(a) !=
             storage.getString(b) ? SYMBOL_TRUE : SYMBOL_FALSE);
    } else {
        push(s,SYMBOL_TRUE);
    }
}

void Engine::opLT() {
    Atom b = pop(s);
    Atom a = pop(s);
    if (isString(a) && isString(b)) {
        push(s,storage.getString(a) <
             storage.getString(b) ? SYMBOL_TRUE : SYMBOL_FALSE);
    } else if (isNumber(a) && isNumber(b)) {
        push(s, storage.getNumber(a) < storage.getNumber(b) ?
                 SYMBOL_TRUE : SYMBOL_FALSE);
    } else {
        push(s,a < b ? SYMBOL_TRUE : SYMBOL_FALSE);
    }
}

void Engine::opLTQ() {
    Atom b = pop(s);
    Atom a = pop(s);
    if (isString(a) && isString(b)) {
        push(s,storage.getString(a) <=
             storage.getString(b) ? SYMBOL_TRUE : SYMBOL_FALSE);
    } else if (isNumber(a) && isNumber(b)) {
        push(s,storage.getNumber(a) <= storage.getNumber(b) ?
                 SYMBOL_TRUE : SYMBOL_FALSE);
    } else {
        push(s,a <= b ? SYMBOL_TRUE : SYMBOL_FALSE);
    }
}

void Engine::opGT() {
    Atom b = pop(s);
    Atom a = pop(s);
    if (isString(a) && isString(b)) {
        push(s,storage.getString(a) >
             storage.getString(b) ? SYMBOL_TRUE : SYMBOL_FALSE);
    } else if (isNumber(a) && isNumber(b)) {
        push(s,storage.getNumber(a) > storage.getNumber(b) ?
                 SYMBOL_TRUE : SYMBOL_FALSE);
    } else {
        push(s,a > b ? SYMBOL_TRUE : SYMBOL_FALSE);
    }
}

void Engine::opGTQ() {
    Atom b = pop(s);
    Atom a = pop(s);
    if (isString(a) && isString(b)) {
        push(s,storage.getString(a) >=
             storage.getString(b) ? SYMBOL_TRUE : SYMBOL_FALSE);
    } else if (isNumber(a) && isNumber(b)) {
        push(s,storage.getNumber(a) >= storage.getNumber(b) ?
                 SYMBOL_TRUE : SYMBOL_FALSE);
    } else {
        push(s,a >= b ? SYMBOL_TRUE : SYMBOL_FALSE);
    }
}

void Engine::opCONCAT() {
    Atom b = pop(s);
    Atom a = pop(s);
    if (isCons(a)) {
        Cons cell = storage.getCons(a);
        while(isCons(cell->cdr)) {
            cell = storage.getCons(cell->cdr);
        }
        if (isCons(b)) {
            cell->cdr = b;
        } else {
            cell->cdr = storage.makeCons(b, NIL);
        }
        push(s, a);
        return;
    }
    if (isCons(b)) {
        push(s, storage.makeCons(a, b));
        return;
    }
    if (isNil(a)) {
        push(s, storage.makeCons(b, NIL));
        return;
    }
    if (isNil(b)) {
        push(s, storage.makeCons(a, NIL));
        return;
    }
    if (isString(a) || isString(b)) {
        push(s, storage.makeString(toSimpleString(a) + toSimpleString(b)));
        return;
    }
    panic(QString("Invalid operands for concat: '")
          + toSimpleString(a)
          + QString("' and '")
          + toSimpleString(b)
          + QString("'"));
}

void Engine::opSCAT() {
    Atom b = pop(s);
    Atom a = pop(s);
    push(s, storage.makeString(toSimpleString(a) + toSimpleString(b)));
}

void Engine::opAND() {
    Atom atomb = pop(s);
    Atom atoma = pop(s);
    push(s, atomb == SYMBOL_TRUE && atoma == SYMBOL_TRUE ?
             SYMBOL_TRUE : SYMBOL_FALSE);
}

void Engine::opOR() {
    Atom atomb = pop(s);
    Atom atoma = pop(s);
    push(s, atomb == SYMBOL_TRUE || atoma == SYMBOL_TRUE ?
             SYMBOL_TRUE : SYMBOL_FALSE);
}

void Engine::opNOT() {
    Atom atom = pop(s);
    push(s, atom == SYMBOL_TRUE ? SYMBOL_FALSE : SYMBOL_TRUE);
}

void Engine::dispatchArithmetic(Atom opcode) {
    Atom atomb = pop(s);
    expect(isNumeric(atomb),
           "Arithmetic: 1st stack top was not a number!",
           __FILE__,
           __LINE__);
    Atom atoma = pop(s);
    expect(isNumeric(atoma),
           "Arithmetic: 2nd stack top was not a number!",
           __FILE__,
           __LINE__);
    if (isNumber(atomb) && isNumber(atoma)) {
        int b = storage.getNumber(atomb);
        int a = storage.getNumber(atoma);
        switch(opcode) {
        case SYMBOL_OP_ADD:
            push(s, storage.makeNumber(a + b));
            return;
        case SYMBOL_OP_MUL:
            push(s, storage.makeNumber(a * b));
            return;
        case SYMBOL_OP_DIV:
            push(s, storage.makeNumber(a / b));
            return;
        case SYMBOL_OP_REM:
            push(s, storage.makeNumber(a % b));
            return;
        case SYMBOL_OP_SUB:
            push(s, storage.makeNumber(a - b));
            return;
        }
    } else {
        double b = 0.0;
        double a = 0.0;
        if (isDecimalNumber(atomb)) {
            b = storage.getDecimal(atomb);
        } else {
            b = static_cast<double>(storage.getNumber(atomb));
        }
        if (isDecimalNumber(atoma)) {
            a = storage.getDecimal(atoma);
        } else {
            a = static_cast<double>(storage.getNumber(atoma));
        }
        switch(opcode) {
        case SYMBOL_OP_ADD:
            push(s, storage.makeDecimal(a + b));
            return;
        case SYMBOL_OP_MUL:
            push(s, storage.makeDecimal(a * b));
            return;
        case SYMBOL_OP_DIV:
            push(s, storage.makeDecimal(a / b));
            return;
        case SYMBOL_OP_REM:
            panic(QString("Cannot compute modulo of decimal values: ") +
                  toSimpleString(atoma) +
                  " and " +
                  toSimpleString(atomb));
            return;
        case SYMBOL_OP_SUB:
            push(s, storage.makeDecimal(a - b));
            return;
        }
    }
}

Atom Engine::locate(Atom pos) {
    expect(isCons(pos),
           "locate: pos is not a pair!",
           __FILE__,
           __LINE__);
    Cons cons = storage.getCons(pos);
    expect(isNumber(cons->car),
           "locate: car is not a number!",
           __FILE__,
           __LINE__);
    Word i = storage.getNumber(cons->car);
    expect(isNumber(cons->car),
           "locate: cdr is not a number!",
           __FILE__,
           __LINE__);
    Word j = storage.getNumber(cons->cdr);
    Atom env = e->atom();
    while (i > 1) {
        if (!isCons(env)) {
            // We could also throw an exception here because this is most
            // likely an error - but we keep hoping and simply return NIL.
            return NIL;
        }
        env = storage.getCons(env)->cdr;
        i--;
    }
    if (!isCons(env)) {
        return NIL;
    }
    env = storage.getCons(env)->car;
    while (j > 1) {
        if (!isCons(env)) {
            // This is probably not an error, but a read on a not yet defined
            // local variable.
            return NIL;
        }
        env = storage.getCons(env)->cdr;
        j--;
    }
    if (!isCons(env)) {
        return NIL;
    }
    return storage.getCons(env)->car;
}

void Engine::store(Atom pos, Atom value) {
    expect(isCons(pos),
           "store: pos is not a pair!",
           __FILE__,
           __LINE__);
    Cons cons = storage.getCons(pos);
    expect(isNumber(cons->car),
           "store: car is not a number!",
           __FILE__,
           __LINE__);
    Word i = storage.getNumber(cons->car);
    expect(isNumber(cons->car),
           "store: cdr is not a number!",
           __FILE__,
           __LINE__);
    Word j = storage.getNumber(cons->cdr);
    Atom env = e->atom();
    while (i > 1) {
        if (!isCons(env)) {
            // We could also throw an exception here because this is most
            // likely an error - but we keep hoping and simply return NIL.
            return;
        }
        env = storage.getCons(env)->cdr;
        i--;
    }
    if (!isCons(env)) {
        return;
    }
    Atom targetEnv = storage.getCons(env)->car;
    if (isNil(targetEnv)) {
        storage.getCons(env)->car = storage.makeCons(NIL, NIL);
    }
    env = storage.getCons(env)->car;
    while (j > 1) {
        if (!isCons(env)) {
            return;
        }
        Cons c = storage.getCons(env);
        if (isNil(c->cdr)) {
            c->cdr = storage.makeCons(NIL, NIL);
        }
        env = c->cdr;
        j--;
    }
    if (!isCons(env)) {
        return;
    }
    storage.getCons(env)->car = value;

}

void Engine::opLine() {
    Atom line = pop(c);
    expect(isNumber(line),
           "#LINE: code top is not a number!",
           __FILE__,
           __LINE__);
    currentLine = storage.getNumber(line);
}

void Engine::opFile() {
    Atom symbol = pop(c);
    expect(isSymbol(symbol),
           "#FILE: code top is not a symbol!",
           __FILE__,
           __LINE__);
    currentFile = symbol;
}

void Engine::dispatch(Atom opcode) {
    instructionCounter++;
    switch (opcode) {
    case SYMBOL_OP_NIL:
        opNIL();
        return;
    case SYMBOL_OP_LDC:
        opLDC();
        return;
    case SYMBOL_OP_LD:
        opLD();
        return;
    case SYMBOL_OP_ST:
        opST();
        return;
    case SYMBOL_OP_LDG:
        opLDG();
        break;
    case SYMBOL_OP_STG:
        opSTG();
        break;
    case SYMBOL_OP_BT:
        opBT();
        return;
    case SYMBOL_OP_LDF:
        opLDF();
        return;
    case SYMBOL_OP_AP0:
        opAP(false);
        return;
    case SYMBOL_OP_AP:
        opAP(true);
        return;
    case SYMBOL_OP_RTN:
        opRTN();
        return;
    case SYMBOL_OP_EQ:
        opEQ();
        return;
    case SYMBOL_OP_NE:
        opNE();
        return;
    case SYMBOL_OP_LT:
        opLT();
        return;
    case SYMBOL_OP_LTQ:
        opLTQ();
        return;
    case SYMBOL_OP_GT:
        opGT();
        return;
    case SYMBOL_OP_GTQ:
        opGTQ();
        return;
    case SYMBOL_OP_SCAT:
        opSCAT();
        return;
    case SYMBOL_OP_CONCAT:
        opCONCAT();
        return;
    case SYMBOL_OP_ADD:
    case SYMBOL_OP_SUB:
    case SYMBOL_OP_DIV:
    case SYMBOL_OP_MUL:
    case SYMBOL_OP_REM:
        dispatchArithmetic(opcode);
        return;
    case SYMBOL_OP_AND:
        opAND();
        return;
    case SYMBOL_OP_OR:
        opOR();
        return;
    case SYMBOL_OP_NOT:
        opNOT();
        return;
    case SYMBOL_OP_CAR:
        opCAR();
        return;
    case SYMBOL_OP_CDR:
        opCDR();
        return;
    case SYMBOL_OP_CONS:
        opCONS();
        return;
    case SYMBOL_OP_SPLIT:
        opSPLIT();
        return;
    case SYMBOL_OP_CHAIN:
        opCHAIN();
        return;
    case SYMBOL_OP_CHAIN_END:
        opCHAINEND();
        return;
    case SYMBOL_OP_FILE:
        opFile();
        return;
    case SYMBOL_OP_LINE:
        opLine();
        return;
    default:
        panic(QString("Invalid op-code: ")+toString(opcode));
        return;
    }
}


bool Engine::loadNextExecution() {
    s->atom(NIL);
    e->atom(NIL);
    c->atom(NIL);
    d->atom(NIL);
    p->atom(NIL);

    if (executionStack.empty()) {
        return false;
    }
    Execution exe = executionStack.front();
    executionStack.pop_front();

    c->atom(exe.fn->atom());
    delete exe.fn;
    currentFile = storage.makeSymbol(exe.filename);
    currentLine = 1;
    push(p, storage.makeCons(currentFile, storage.makeNumber(currentLine)));

    return true;
}

bool Engine::isRunnable() {
    return running;
}

void Engine::eval(const QString& source, const QString& filename) {
    Atom code = compileSource(filename, source, true, false);
    Execution exe;
    exe.filename = filename;
    exe.fn = storage.ref(code);
    executionStack.push_back(exe);
    if (!running) {
        running = true;
        emit onEngineStarted();
    }}

void Engine::interpret() {
    if (!running) {
        return;
    }
    if (c->atom() == NIL) {
        if (!loadNextExecution()) {
            running = false;
            emit onEngineStopped();
            return;
        }
    }
    try {
        Word maxOpCodes = opCodesInInterpret;
        while (running && maxOpCodes > 0) {
            Atom op = pop(c);
            if (op == SYMBOL_OP_STOP) {
                gc();
                if (!loadNextExecution()) {
                    running = false;
                    emit onEngineStopped();
                    return;
                }
            } else {
                dispatch(op);
            }
            maxOpCodes--;
        }
    } catch(PanicException* ex) {
        running = false;
        emit onEngineStopped();
        executionStack.clear();
        emit onEnginePanic(currentFile, currentLine, lastError, stackDump());
    }
}

void Engine::expect(bool expectation,
                    QString errorMessage,
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

QString Engine::stackDump() {
    QString buffer;
    buffer += QString("Error:\n");
    buffer += "--------------------------------------------\n";
    buffer += "Stacktrace:\n";
    buffer += "--------------------------------------------\n";
    buffer += toSimpleString(currentFile) +
              ":" +
              intToString(currentLine) +
              "\n";
    Atom pos = pop(p);
    while(isCons(pos)) {
        Cons location = storage.getCons(pos);
        buffer += toSimpleString(location->car) +
                  ":" +
                  toSimpleString(location->cdr) +
                  "\n";
        pos = pop(p);
    }
    buffer += "\n";

    buffer += "Registers:\n";
    buffer += "--------------------------------------------\n";
    buffer += "S: " + toString(s->atom()) + "\n";
    buffer += "E: " + toString(e->atom()) + "\n";
    buffer += "C: " + toString(c->atom()) + "\n";
    buffer += "D: " + toString(d->atom()) + "\n";

    return buffer;
}

void Engine::panic(const QString& error) {
    lastError = error;
    throw new PanicException();
}

QSettings* Engine::getSettings() {
    return settings;
}

QString Engine::lookupSource(const QString& fileName) {
    QDir currentDir;
    QFileInfo file = QFileInfo(currentDir.absolutePath()
                               + "/"
                               + fileName);
    if (file.exists()) {
        return file.absoluteFilePath();
    }
    file = QFileInfo(homeDir.absolutePath()
                     + "/"
                     + fileName);
    expect(file.exists(), QString("File '%1' was not found! Paths: '%2', '%3'").
           arg(fileName,
               currentDir.absolutePath(),
               homeDir.absolutePath()), __FILE__, __LINE__);
    return file.absoluteFilePath();
}

Atom Engine::compileFile(const QString& file, bool insertStop) {
    QString path = lookupSource(file);
    QFile f(path);
    if (f.open(QFile::ReadOnly | QFile::Text)) {
        return compileSource(path, f.readAll(), insertStop, false);
    } else {
        panic(QString("Cannot compile: ") +
              path +
              QString(". File cannot be opened!"));
        return NIL;
    }
}

Atom Engine::compileSource(const QString& file,
                           const QString& source,
                           bool insertStop,
                           bool silent)
{
    Compiler compiler(file, source, this);
    std::pair<Atom, std::vector<CompilationError> >
            result = compiler.compile(insertStop);
    if (!result.second.empty()) {
        if (!silent) {
            QString buf;
            buf += "Compilation error(s) in: " + file + "\n";
            for(std::vector<CompilationError>::iterator
                i = result.second.begin();
                i != result.second.end();
                i++)
            {
                CompilationError e = *i;
                buf += intToString(e.line)
                       + ":"
                       + intToString(e.pos)
                       + ": "
                       + e.error
                       + "\n";
            }
            println(buf);
        }
        return NIL;
    }
    return result.first;
}

void Engine::call(Atom list) {
    if (!isCons(list)) {
        return;
    }
    push(d, e->atom());
    push(d, s->atom());
    push(d, c->atom());
    s->atom(NIL);
    //Check if code ends with an RTN statement...
    Atom tmp = list;
    Cons cell = storage.getCons(tmp);
    while(true) {
        if (isCons(cell->cdr)) {
            tmp = cell->cdr;
            cell = storage.getCons(tmp);
        } else {
            break;
        }
    }
    //If not, append an RTN.
    if (cell->car != SYMBOL_OP_RTN) {
        cell->cdr = storage.makeCons(SYMBOL_OP_RTN, NIL);
    }
    c->atom(list);
    push(d, c->atom());
    e->atom(NIL);
    push(p, storage.makeCons(currentFile, storage.makeNumber(currentLine)));
}


void Engine::println(const QString& string) {
    emit onLog(string);
}

QString Engine::printList(Atom atom) {
    QString sb("");
    Cons cons = storage.getCons(atom);
    sb += QString("(") + toString(cons->car);
    if (isCons(cons->cdr) || isNil(cons->cdr)) {
        Atom val = cons->cdr;
        while (isCons(val)) {
            cons = storage.getCons(val);
            sb += " " + toString(cons->car);
            val = cons->cdr;
        }
        if (!isCons(val) && !isNil(val)) {
            sb += " " + toString(val);
        }
    } else {
        sb += "." + toString(cons->cdr);
    }
    sb += ")";
    return sb;
}

QString Engine::toString(Atom atom) {
    if (isNil(atom)) {
        return QString("NIL");
    }
    Word type = getType(atom);
    std::stringstream sb;
    switch(type) {
    case TAG_TYPE_NUMBER:
    case TAG_TYPE_LARGE_NUMBER:
        sb << storage.getNumber(atom);
        return QString::fromStdString(sb.str());
    case TAG_TYPE_DECIMAL_NUMBER:
        sb << storage.getDecimal(atom);
        return QString::fromStdString(sb.str());
    case TAG_TYPE_BIF:
        return QString("$") + getBIFName(atom);
    case TAG_TYPE_GLOBAL:
        return QString("@") + storage.getGlobalName(atom);
    case TAG_TYPE_STRING:
        return QString("'") + storage.getString(atom) + QString("'");
    case TAG_TYPE_SYMBOL:
        return QString("#") + storage.getSymbolName(atom);
    case TAG_TYPE_CONS:
        return printList(atom);
    case TAG_TYPE_REFERENCE:
        return storage.getReference(atom)->toString();
    default:
        return QString("UNKNOWN");
    }
}

QString Engine::toSimpleString(Atom atom) {
    if (isNil(atom)) {
        return QString("");
    }
    Word type = getType(atom);
    std::stringstream sb;
    switch(type) {
    case TAG_TYPE_NUMBER :
    case TAG_TYPE_LARGE_NUMBER:
        sb << storage.getNumber(atom);
        return QString::fromStdString(sb.str());
    case TAG_TYPE_DECIMAL_NUMBER:
        sb << storage.getDecimal(atom);
        return QString::fromStdString(sb.str());
    case TAG_TYPE_BIF:
        return getBIFName(atom);
    case TAG_TYPE_GLOBAL:
        return storage.getGlobalName(atom);
    case TAG_TYPE_STRING:
        return storage.getString(atom);
    case TAG_TYPE_SYMBOL:
        return storage.getSymbolName(atom);
    case TAG_TYPE_CONS:
        return printList(atom);
    case TAG_TYPE_REFERENCE:
        return storage.getReference(atom)->toString();
    default:
        return QString("");
    }
}

void Engine::initializeBIF() {
    CoreExtension::INSTANCE->registerBuiltInFunctions(this);
    FilesExtension::INSTANCE->registerBuiltInFunctions(this);
    WebExtension::INSTANCE->registerBuiltInFunctions(this);
}

void Engine::setValue(Atom name, Atom value) {
    if (name == SYMBOL_VALUE_OP_CODES_PER_EVENT_LOOP) {
        if (!isNumber(value)) {
            panic(QString("setValue requires a number for the key %1 (%2:%3)").
                  arg(toSimpleString(name), __FILE__, intToString(__LINE__)));
        }
        opCodesInInterpret = std::max(1l, storage.getNumber(value));
        settings->setValue(toSimpleString(SYMBOL_VALUE_OP_CODES_PER_EVENT_LOOP),
                           opCodesInInterpret);
        settings->sync();
    }
}

Atom Engine::getValue(Atom name) {
    if (name == SYMBOL_VALUE_OP_COUNT) {
        return storage.makeNumber(instructionCounter);
    } else if (name == SYMBOL_VALUE_GC_COUNT) {
        return storage.makeNumber(storage.statusNumGC());
    } else if (name == SYMBOL_VALUE_OP_CODES_PER_EVENT_LOOP) {
        return storage.makeNumber(opCodesInInterpret);
    } else if (name == SYMBOL_VALUE_NUM_GC_ROOTS) {
        return storage.makeNumber(storage.statusNumGCRoots());
    } else if (name == SYMBOL_VALUE_NUM_SYMBOLS) {
        return storage.makeNumber(storage.statusNumSymbols());
    } else if (name == SYMBOL_VALUE_NUM_GLOBALS) {
        return storage.makeNumber(storage.statusNumGlobals());
    } else if (name == SYMBOL_VALUE_NUM_TOTAL_CELLS) {
        return storage.makeNumber(storage.statusTotalCells());
    } else if (name == SYMBOL_VALUE_NUM_CELLS_USED) {
        return storage.makeNumber(storage.statusCellsUsed());
    } else if (name == SYMBOL_VALUE_NUM_TOTAL_STRINGS) {
        return storage.makeNumber(storage.statusTotalStrings());
    } else if (name == SYMBOL_VALUE_NUM_STRINGS_USED) {
        return storage.makeNumber(storage.statusStringsUsed());
    } else if (name == SYMBOL_VALUE_NUM_TOTAL_NUMBERS) {
        return storage.makeNumber(storage.statusTotalNumbers());
    } else if (name == SYMBOL_VALUE_NUM_NUMBERS_USED) {
        return storage.makeNumber(storage.statusNumbersUsed());
    } else if (name == SYMBOL_VALUE_NUM_TOTAL_DECIMALS) {
        return storage.makeNumber(storage.statusTotalDecimals());
    } else if (name == SYMBOL_VALUE_NUM_DECIMALS_USED) {
        return storage.makeNumber(storage.statusDecimalsUsed());
    } else if (name == SYMBOL_VALUE_NUM_TOTAL_REFERENCES) {
        return storage.makeNumber(storage.statusTotalReferences());
    } else if (name == SYMBOL_VALUE_NUM_REFERENES_USED) {
        return storage.makeNumber(storage.statusReferencesUsed());
    }

    return NIL;
}
