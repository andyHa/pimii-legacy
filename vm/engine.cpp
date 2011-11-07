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

#include <QFile>
#include <QPluginLoader>

#include <sstream>
#include <exception>

Logger Engine::log("EXEC");

void Engine::push(AtomRef* reg, Atom atom) {
   reg->atom(storage.makeCons(atom, reg->atom()));
}

Atom Engine::pop(AtomRef* reg) {
    if (!isCons(reg->atom())) {
        return NIL;
    }
    Cell cons = storage.getCons(reg->atom());
    Atom result = cons.car;
    reg->atom(cons.cdr);
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
   AtomRef val(&storage, pop(s));
   store(pop(c), val.atom());
   push(s, val.atom());
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
        list = storage.getCons(list).cdr;
    }
    if (isCons(list)) {
        return storage.getCons(list).car;
    } else {
        return NIL;
    }
}

Atom Engine::head(AtomRef* list) {
    if (isCons(list->atom())) {
       return storage.getCons(list->atom()).car;
    } else {
        return NIL;
    }
}

void Engine::opAP(bool hasArguments) {
    AtomRef name(&storage, pop(c));
    AtomRef fun(&storage, pop(s));
    AtomRef v(&storage, NIL);
    if (hasArguments) {
        v.atom(pop(s));
    }
    if (isBIF(fun.atom())) {
        BIF bif = getBuiltInFunction(fun.atom());
        CallContext ctx(this, &storage, v.atom());
        bif(ctx);
        push(s, ctx.getResult());
    } else {
        if (!isCons(fun.atom())) {
         panic(
           QString("'%1' is neither a closure nor a built in function (%2:%3)")
                        .arg(storage.getSymbolName(name.atom()),
                             QString(__FILE__),
                             intToString(__LINE__)));
        }
        Cell funPair = storage.getCons(fun.atom());
        if ((head(c) == SYMBOL_OP_RTN) && (funPair.car == head(d))) {
            // We have a tail recursion -> don't push useless stuff on the
            // dump-stack, only flush stack, restart code and environment
            // (with new args)
            s->atom(NIL);
            c->atom(funPair.car);
            e->atom(storage.makeCons(v.atom(), funPair.cdr));
        } else {
            push(d, e->atom());
            push(d, s->atom());
            push(d, c->atom());
            s->atom(NIL);
            c->atom(funPair.car);
            push(d, c->atom());
            e->atom(storage.makeCons(v.atom(), funPair.cdr));
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
    Atom pos = pop(p);
    if (isCons(pos)) {
        Cell cell = storage.getCons(pos);
        currentFile = cell.car;
        currentLine = storage.getNumber(cell.cdr);
    }
}

void Engine::opCAR() {
    Atom atom = pop(s);
    expect(isCons(atom),
           "#CAR: stack top was not a cons!",
           __FILE__,
           __LINE__);
    Cell cons = storage.getCons(atom);
    push(s, cons.car);
}

void Engine::opCDR() {
    Atom atom = pop(s);
    expect(isCons(atom),
           "#CDR: stack top was not a cons!",
           __FILE__,
           __LINE__);
    Cell cons = storage.getCons(atom);
    push(s, cons.cdr);
}

void Engine::opCONS() {
    Atom b = pop(s);
    Atom a = pop(s);
    push(s, storage.makeCons(a,b));
}

void Engine::opSPLIT() {
    AtomRef element(&storage, pop(s));
    AtomRef l1(&storage, pop(c));
    AtomRef l2(&storage, pop(c));
    if (isCons(element.atom())) {
        Cell c = storage.getCons(element.atom());
        if (isGlobal(l1.atom())) {
            storage.writeGlobal(l1.atom(), c.car);
        } else if (isCons(l1.atom())) {
            store(l1.atom(), c.car);
        }
        if (isGlobal(l2.atom())) {
            storage.writeGlobal(l2.atom(), c.cdr);
        } else if (isCons(l2.atom())) {
            store(l2.atom(), c.cdr);
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
        Cell c = storage.getCons(cell);
        storage.setCDR(cell, storage.append(c.cdr, element));
        push(s, cell);
    }
}

void Engine::opCHAINEND() {
    Atom cell = pop(s);
    if (!isCons(cell)) {
        push(s, storage.makeCons(cell, NIL));
    } else {
        Cell c = storage.getCons(cell);
        push(s, c.car);
    }
}

Relation Engine::compare(Atom a, Atom b) {
    if (getType(a) != getType(b)) {
        return NE;
    }
    if (isString(a) && isString(b)) {
        return compareStrings(a, b);
    } else if (isNumeric(a) && isNumeric(b)) {
        return compareNumerics(a, b);
    } else if (isCons(a) && isCons(b)) {
        return compareLists(a, b);
    }
    // Directly compare by atom index.
    if (a < b) {
        return LT;
    } else if (a > b) {
        return GT;
    } else {
        return EQ;
    }
}

Relation Engine::compareLists(Atom a, Atom b) {
    while(isCons(a) && isCons(b)) {
        Cell ca = storage.getCons(a);
        Cell cb = storage.getCons(b);
        Relation result = compare(ca.car, cb.car);
        if (result != EQ) {
            return result;
        }
        a = ca.cdr;
        b = cb.cdr;
    }
    if (isNil(a) && !isNil(b)) {
        return LT;
    } else if (!isNil(a)) {
        return GT;
    } else {
        return EQ;
    }
}

Relation Engine::compareStrings(Atom a, Atom b) {
    QString as = storage.getString(a);
    QString bs = storage.getString(b);
    if (as < bs) {
        return LT;
    } else if (as > bs) {
        return GT;
    } else {
        return EQ;
    }
}

Relation Engine::compareNumerics(Atom a, Atom b) {
    double da;
    double db;
    convertNumeric(a, b, &da, &db);
    if (fabs(da - db) <= DOUBLE_EQUALITY_EPSILON) {
        return EQ;
    }
    if (da < db) {
        return LT;
    } else {
        return GT;
    }
}

void Engine::opEQ() {
    Atom b = pop(s);
    Atom a = pop(s);
    push(s, compare(a, b) == EQ ? SYMBOL_TRUE : SYMBOL_FALSE);
}

void Engine::opNE() {
    Atom b = pop(s);
    Atom a = pop(s);
    push(s, compare(a, b) != EQ ? SYMBOL_TRUE : SYMBOL_FALSE);
}

void Engine::opLT() {
    Atom b = pop(s);
    Atom a = pop(s);
    Relation result = compare(a, b);
    if (result == NE) {
        panic(QString("Cannot order values of different types: %1, %2").
              arg(toString(a), toString(b)));
    }
    push(s, result == LT ? SYMBOL_TRUE : SYMBOL_FALSE);
}

void Engine::opLTQ() {
    Atom b = pop(s);
    Atom a = pop(s);
    Relation result = compare(a, b);
    if (result == NE) {
        panic(QString("Cannot order values of different types: %1, %2").
              arg(toString(a), toString(b)));
    }
    push(s, result == LT || result == EQ ? SYMBOL_TRUE : SYMBOL_FALSE);
}

void Engine::opGT() {
    Atom b = pop(s);
    Atom a = pop(s);
    Relation result = compare(a, b);
    if (result == NE) {
        panic(QString("Cannot order values of different types: %1, %2").
              arg(toString(a), toString(b)));
    }
    push(s, result == GT ? SYMBOL_TRUE : SYMBOL_FALSE);
}

void Engine::opGTQ() {
    Atom b = pop(s);
    Atom a = pop(s);
    Relation result = compare(a, b);
    if (result == NE) {
        panic(QString("Cannot order values of different types: %1, %2").
              arg(toString(a), toString(b)));
    }
    push(s, result == GT || result == EQ ? SYMBOL_TRUE : SYMBOL_FALSE);

}

void Engine::opCONCAT() {
    Atom b = pop(s);
    Atom a = pop(s);
    if (isCons(a)) {
        Cell cell = storage.getCons(a);
        Atom tail = a;
        while(isCons(cell.cdr)) {
            tail = cell.cdr;
            cell = storage.getCons(cell.cdr);
        }
        if (isCons(b)) {
            storage.setCDR(tail, b);

        } else {
            storage.setCDR(tail, storage.makeCons(b, NIL));
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

void Engine::convertNumeric(Word atoma, Word atomb, double* a, double* b) {
    if (isDecimalNumber(atomb)) {
        *b = storage.getDecimal(atomb);
    } else {
        *b = static_cast<double>(storage.getNumber(atomb));
    }
    if (isDecimalNumber(atoma)) {
        *a = storage.getDecimal(atoma);
    } else {
        *a = static_cast<double>(storage.getNumber(atoma));
    }
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
        long b = storage.getNumber(atomb);
        long a = storage.getNumber(atoma);
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
        double a;
        double b;
        convertNumeric(atoma, atomb, &a, &b);

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
    Cell cons = storage.getCons(pos);
    expect(isNumber(cons.car),
           "locate: car is not a number!",
           __FILE__,
           __LINE__);
    long i = storage.getNumber(cons.car);
    expect(isNumber(cons.car),
           "locate: cdr is not a number!",
           __FILE__,
           __LINE__);
    long j = storage.getNumber(cons.cdr);
    Atom env = e->atom();
    while (i > 1) {
        if (!isCons(env)) {
            // We could also throw an exception here because this is most
            // likely an error - but we keep hoping and simply return NIL.
            return NIL;
        }
        env = storage.getCons(env).cdr;
        i--;
    }
    if (!isCons(env)) {
        return NIL;
    }
    env = storage.getCons(env).car;
    while (j > 1) {
        if (!isCons(env)) {
            // This is probably not an error, but a read on a not yet defined
            // local variable.
            return NIL;
        }
        env = storage.getCons(env).cdr;
        j--;
    }
    if (!isCons(env)) {
        return NIL;
    }
    return storage.getCons(env).car;
}

void Engine::store(Atom pos, Atom value) {
    expect(isCons(pos),
           "store: pos is not a pair!",
           __FILE__,
           __LINE__);
    Cell cons = storage.getCons(pos);
    expect(isNumber(cons.car),
           "store: car is not a number!",
           __FILE__,
           __LINE__);
    long i = storage.getNumber(cons.car);
    expect(isNumber(cons.car),
           "store: cdr is not a number!",
           __FILE__,
           __LINE__);
    long j = storage.getNumber(cons.cdr);
    AtomRef env(&storage, e->atom());
    while (i > 1) {
        if (!isCons(env.atom())) {
            // We could also throw an exception here because this is most
            // likely an error - but we keep hoping and simply return NIL.
            return;
        }
        env.atom(storage.getCons(env.atom()).cdr);
        i--;
    }
    if (!isCons(env.atom())) {
        return;
    }
    if (isNil(storage.getCons(env.atom()).car)) {
        storage.setCAR(env.atom(), storage.makeCons(NIL, NIL));
    }
    env.atom(storage.getCons(env.atom()).car);
    while (j > 1) {
        if (!isCons(env.atom())) {
            return;
        }
        Cell c = storage.getCons(env.atom());
        if (isNil(c.cdr)) {
            storage.setCDR(env.atom(), storage.makeCons(NIL, NIL));
            // Refresh cell...
            c = storage.getCons(env.atom());
        }
        env.atom(c.cdr);
        j--;
    }
    if (!isCons(env.atom())) {
        return;
    }
    storage.setCAR(env.atom(), value);
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
    c->atom(exe.fn->atom());

    return true;
}

bool Engine::isRunnable() {
    return running;
}

void Engine::eval(const QString& source,
                  const QString& filename,
                  bool printStackTop)
{
    Atom code = compileSource(filename, source, true, false);
    if (code != NIL) {
        Execution exe;
        exe.filename = filename;
        exe.fn = storage.ref(code);
        exe.printStackTop = printStackTop;
        executionStack.push_back(exe);
        if (!running) {
            running = true;
            emit onEngineStarted();
        }
    }
}

void Engine::interpret() {
    if (!running) {
        return;
    }
    TRACE(log, "Entering Interpert...");
    if (c->atom() == NIL) {
        TRACE(log, "Loading next execution...");
        if (!loadNextExecution()) {
            TRACE(log, "Nothing to do. Halting engine.");
            running = false;
            emit onEngineStopped();
            return;
        }
    }
    try {
        Word maxOpCodes = TUNING_PARAM_MAX_OP_CODES_IN_INTERPRET;
        while (running && maxOpCodes > 0) {
            Atom op = pop(c);
            if (op == SYMBOL_OP_STOP) {
                Execution exe = executionStack.front();
                // Check if we should print the execution result...
                if (exe.printStackTop) {
                    INFO(log, toString(pop(s)));
                }

                // Remove execution...
                executionStack.pop_front();
                delete exe.fn;

                TRACE(log, "STOP requested. Loading next execution.");
                if (!loadNextExecution()) {
                    TRACE(log, "Nothing to do. Halting engine.");
                    running = false;
                    emit onEngineStopped();
                    return;
                }
            } else {
                dispatch(op);
            }
            maxOpCodes--;
        }
        TRACE(log, "Leaving interpret...");
    } catch(PanicException* ex) {
        running = false;
        emit onEngineStopped();
        executionStack.clear();
        emit onEnginePanic(currentFile, currentLine, lastError, stackDump());
        c->atom(NIL);
    }
}

QString Engine::stackDump() {
    QString buffer;
    buffer += "Stacktrace:\n";
    buffer += "--------------------------------------------\n";
    buffer += toSimpleString(currentFile) +
              ":" +
              intToString(currentLine) +
              "\n";
    Atom pos = pop(p);
    while(isCons(pos)) {
        Cell location = storage.getCons(pos);
        buffer += toSimpleString(location.car) +
                  ":" +
                  toSimpleString(location.cdr) +
                  "\n";
        pos = pop(p);
    }
    buffer += "\n";

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

    if (!file.exists()) {
        panic(QString("File '%1' was not found! Paths: '%2', '%3'").
              arg(fileName,
                  currentDir.absolutePath(),
                  homeDir.absolutePath()) +
              QString(" (") +
              QString(__FILE__) +
              QString(":") +
              intToString(__LINE__) +
              QString(")"));
    }
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
    if (!compiler.compile(insertStop)) {
        if (!silent) {
            std::vector<CompilationError> errors = compiler.getErrors();
            QString buf;
            buf += "Compilation error(s) in: " + file + "\n";
            for(std::vector<CompilationError>::iterator
                i = errors.begin();
                i != errors.end();
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
            INFO(log, buf);
        }
        return NIL;
    }
    return compiler.getCode();
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
    Cell cell = storage.getCons(tmp);
    while(true) {
        if (isCons(cell.cdr)) {
            tmp = cell.cdr;
            cell = storage.getCons(tmp);
        } else {
            break;
        }
    }
    //If not, append an RTN.
    if (cell.car != SYMBOL_OP_RTN) {
        cell.cdr = storage.makeCons(SYMBOL_OP_RTN, NIL);
    }
    c->atom(list);
    push(d, c->atom());
    e->atom(NIL);
    push(p, storage.makeCons(currentFile, storage.makeNumber(currentLine)));
}

QString Engine::printList(Atom atom) {
    QString sb("");
    Cell cons = storage.getCons(atom);
    sb += QString("(") + toString(cons.car);
    if (isCons(cons.cdr) || isNil(cons.cdr)) {
        Atom val = cons.cdr;
        while (isCons(val)) {
            cons = storage.getCons(val);
            sb += " " + toString(cons.car);
            val = cons.cdr;
        }
        if (!isCons(val) && !isNil(val)) {
            sb += " " + toString(val);
        }
    } else {
        sb += "." + toString(cons.cdr);
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

}

Atom Engine::getValue(Atom name) {
    if (name == SYMBOL_VALUE_OP_COUNT) {
        return storage.makeNumber(instructionCounter);
    } else if (name == SYMBOL_VALUE_GC_COUNT) {
        return storage.makeNumber(storage.statusNumGC());
    } else if (name == SYMBOL_VALUE_GC_EFFICIENCY) {
        return storage.makeDecimal(storage.statusGCEfficienty());
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
    } else if (name == SYMBOL_VALUE_NUM_REFERENCES_USED) {
        return storage.makeNumber(storage.statusReferencesUsed());
    } else if (name == SYMBOL_VALUE_HOME_PATH) {
        return storage.makeString(homeDir.absolutePath());
    }

    return NIL;
}
