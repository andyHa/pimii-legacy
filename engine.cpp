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
  see engine.h
 */

#include "engine.h"
#include "tools.h"
#include "compiler.h"
#include "bytecodeparser.h"

#include <cassert>
#include <fstream>

void Engine::push(Atom& reg, Atom atom) {
   reg = storage.makeCons(atom, reg);
}

Atom Engine::pop(Atom& reg) {
    if (!isCons(reg)) {
        return NIL;
    }
    Cons cons = storage.getCons(reg);
    Atom result = cons->car;
    reg = cons->cdr;
    return result;
}

Engine::Engine() {
    initializeBIF();
}

Engine::~Engine() {

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

Atom Engine::findBuiltInFunction(Atom nameSymbol) {
    expect(isSymbol(nameSymbol),
           "nameSymbol is not a symbol",
           __FILE__,
           __LINE__);
    Word index;
    if (!bifTable.find(nameSymbol, index)) {
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

String Engine::getBIFName(Atom atom) {
    expect(isBIF(atom),
           "given atom is not a built in function",
           __FILE__,
           __LINE__);
    return storage.getSymbolName(bifTable.getKey(untagIndex(atom)));
}

bool Engine::shouldGC() {
    return false;
}

void Engine::gc() {
    storage.gcBegin();
    storage.addGCRoot(s);
    storage.addGCRoot(e);
    storage.addGCRoot(c);
    storage.addGCRoot(d);
    storage.gcComplete();
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
   store(pop(c), pop(s));
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
    expect(isGlobal(gobal),
           "#STG: code top was not a global",
           __FILE__,
           __LINE__);
    storage.writeGlobal(gobal, pop(s));
}

void Engine::opSEL() {
    Atom discriminator = pop(s);
    Atom ct = pop(c);
    Atom cf = pop(c);
    push(d, c);
    if (discriminator == SYMBOL_TRUE) {
        c = ct;
    } else {
        c = cf;
    }
}

void Engine::opJOIN() {
    c = pop(d);
}

void Engine::opLDF() {
    push(s, storage.makeCons(pop(c), e));
}

void Engine::opAP() {
    Atom fun = pop(s);
    Atom v = pop(s);
    if (isBIF(fun)) {
        BIF bif = getBuiltInFunction(fun);
        push(s, bif(this, storage, v));
    } else {
        expect(isCons(fun),
               "#AP: code top was neither a built in function or a closure!",
               __FILE__,
               __LINE__);
        Cons funPair = storage.getCons(fun);
        push(d, c);
        push(d, e);
        push(d, s);
        s = NIL;
        c = funPair->car;
        e = storage.makeCons(v, funPair->cdr);
        push(p, storage.makeCons(currentFile, makeNumber(currentLine)));
    }
}

void Engine::opRTN() {
    Atom result = pop(s);
    s = pop(d);
    push(s, result);
    e = pop(d);
    c = pop(d);
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

void Engine::opRPLACAR() {
    Atom element = pop(s);
    Atom cell = pop(s);
    expect(isCons(cell),
           "#RPLACAR: stack top was not a cons!",
           __FILE__,
           __LINE__);
    Cons c = storage.getCons(cell);
    c->car = element;
    push(s, cell);
}

void Engine::opRPLACDR() {
    Atom element = pop(s);
    Atom cell = pop(s);
    expect(isCons(cell),
           "#RPLACDR: stack top was not a cons!",
           __FILE__,
           __LINE__);
    Cons c = storage.getCons(cell);
    c->cdr = element;
    push(s, cell);
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
    } else {
        push(s,a >= b ? SYMBOL_TRUE : SYMBOL_FALSE);
    }
}

void Engine::opADD() {
    std::wcerr << toString(s) << std::endl;
    Atom b = pop(s);
    std::wcerr << toString(s) << std::endl;
    Atom a = pop(s);
    std::wcerr << toString(s) << std::endl;
    std::wcerr << toString(b) << std::endl;
    std::wcerr << toString(a) << std::endl;
    if (isNumber(b) && isNumber(a)) {
        push(s, makeNumber(getNumber(a) + getNumber(b)));
        return;
    }
    if (isString(a) || isString(b)) {
        push(s, storage.makeString(toSimpleString(a) + toSimpleString(b)));
        return;
    }
    panic(String(L"Invalid operands for addition: '")
          + toSimpleString(a)
          + String(L"' and '")
          + toSimpleString(b)
          + String(L"'"));
}

void Engine::opSUB(int a, int b) {
    push(s, makeNumber(a - b));
}

void Engine::opMUL(int a, int b) {
    push(s, makeNumber(a * b));
}

void Engine::opDIV(int a, int b) {
    push(s, makeNumber(a / b));
}

void Engine::opREM(int a, int b) {
    push(s, makeNumber(a % b));
}

void Engine::dispatchArithmetic(Atom opcode) {
    Atom atomb = pop(s);
    expect(isNumber(atomb),
           "Arithmetic: 1st stack top was not a number!",
           __FILE__,
           __LINE__);
    Atom atoma = pop(s);
    expect(isNumber(atoma),
           "Arithmetic: 2nd stack top was not a number!",
           __FILE__,
           __LINE__);
    int b = getNumber(atomb);
    int a = getNumber(atoma);
    switch(opcode) {
    case SYMBOL_OP_MUL:
        opMUL(a, b);
        return;
    case SYMBOL_OP_DIV:
        opDIV(a, b);
        return;
    case SYMBOL_OP_REM:
        opREM(a, b);
        return;
    case SYMBOL_OP_SUB:
        opSUB(a, b);
        return;
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
    Word i = getNumber(cons->car);
    expect(isNumber(cons->car),
           "locate: cdr is not a number!",
           __FILE__,
           __LINE__);
    Word j = getNumber(cons->cdr);
    Atom env = e;
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
    Word i = getNumber(cons->car);
    expect(isNumber(cons->car),
           "store: cdr is not a number!",
           __FILE__,
           __LINE__);
    Word j = getNumber(cons->cdr);
    Atom env = e;
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
    currentLine = getNumber(line);
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
    case SYMBOL_OP_SEL:
        opSEL();
        return;
    case SYMBOL_OP_JOIN:
       opJOIN();
       return;
    case SYMBOL_OP_LDF:
        opLDF();
        return;
    case SYMBOL_OP_AP:
        opAP();
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
    case SYMBOL_OP_ADD:
        opADD();
        return;
    case SYMBOL_OP_SUB:
    case SYMBOL_OP_DIV:
    case SYMBOL_OP_MUL:
    case SYMBOL_OP_REM:
        dispatchArithmetic(opcode);
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
    case SYMBOL_OP_RPLACAR:
        opRPLACAR();
        return;
    case SYMBOL_OP_RPLACDR:
        opRPLACDR();
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
        panic(String(L"Invalid op-code: ")+toString(opcode));
        return;
    }
}

Atom Engine::exec(String filename, Atom code) {
    s = NIL;
    e = NIL;
    c = code;
    d = NIL;
    p = NIL;
    currentFile = storage.makeSymbol(filename);
    currentLine = 1;
    push(p, storage.makeCons(currentFile,  makeNumber(currentLine)));

    try {
        try {
            while (true) {
                Atom op = pop(c);
                if (op == SYMBOL_OP_STOP) {
                    return pop(s);
                } else {
                    dispatch(op);
                    if (shouldGC()) {
                        gc();
                    }
                }
            }
        } catch(StopEngineException* e) {
            //Error is already handled...
            return NIL;
        } catch(std::exception* e) {
            std::wstringstream ss;
            ss << e->what();
            panic(ss.str());
        }
    } catch(StopEngineException* e) {
        //Error is already handled...
        return NIL;
    }

    return NIL; // unreachable
}

void Engine::kickstart(String filename) {
    try {
        s = NIL;
        e = NIL;
        c = NIL;
        d = NIL;
        p = NIL;
        currentFile = storage.makeSymbol(String(L"kickstarter"));
        currentLine = 1;
        push(p, storage.makeCons(currentFile, makeNumber(currentLine)));
        Atom code = compileFile(filename, true);
        std::wcerr << toString(code) << std::endl;
        exec(filename, code);
    } catch(StopEngineException* e) {
        //Error is already handled...
    }
}

void Engine::expect(bool expectation, const char* errorMessage, const char* file, int line) {
    if (!expectation) {
        std::wstringstream str;
        str << errorMessage << " (" << file << ":" << line << ")";
        panic(str.str());
    }
}

void Engine::panic(String error) {
    std::wcerr << "Error:" << std::endl;
    std::wcerr << "--------------------------------------------" << std::endl;
    std::wcerr << error << std::endl << std::endl;
    std::wcerr << "Stacktrace:" << std::endl;
    std::wcerr << "--------------------------------------------" << std::endl;
    std::wcerr << toSimpleString(currentFile) << ":" << currentLine << std::endl;
    Atom pos = pop(p);
    while(isCons(pos)) {
        Cons location = storage.getCons(pos);
        std::wcerr << toSimpleString(location->car) << ":" << toSimpleString(location->cdr) << std::endl;
        pos = pop(p);
    }
    std::wcerr << std::endl;
    std::wcerr << "Registers:" << std::endl;
    std::wcerr << "--------------------------------------------" << std::endl;
    std::wcerr << "S: " << toString(s) << std::endl;
    std::wcerr << "E: " << toString(e) << std::endl;
    std::wcerr << "C: " << toString(c) << std::endl;
    std::wcerr << "D: " << toString(d) << std::endl << std::endl;

    // Stop engine...
    throw new StopEngineException();
}

String Engine::lookupSource(String fileName) {
    for(std::vector<String>::iterator i = sourcePaths.begin(); i != sourcePaths.end(); i++) {
        std::string path = asStdString((*i) + fileName);
        std::wifstream is(path.c_str(), std::ios::in);
        if (is) {
            return (*i) + fileName;
        }
    }
    return fileName;
}

void Engine::addSourcePath(String path) {
    sourcePaths.insert(sourcePaths.begin(), path);
}

Atom Engine::compileFile(String file, bool insertStop) {
    String path = lookupSource(file);
    std::wifstream stream(asStdString(path).c_str(), std::ios::in);
    if (!stream) {
        panic(String(L"Cannot compile: ") + file + String(L". File was not found!"));
        return NIL;
    }
    Compiler compiler(file, stream, this);
    std::pair<Atom, std::vector<CompilationError> > result = compiler.compile(insertStop);
    if (!result.second.empty()) {
        std::wstringstream buf;
        buf << "Compilation error(s) in: " << file << std::endl;
        for(std::vector<CompilationError>::iterator i = result.second.begin(); i != result.second.end(); i++) {
            CompilationError e = *i;
            buf << e.line << ":" << e.pos << ": " << e.error << std::endl;
        }
        panic(buf.str());
        return NIL;
    }
    return result.first;
}

void Engine::print(String string) {
    std::wcout << string;
}

void Engine::newLine() {
    std::wcout << std::endl;
}

String Engine::printList(Atom atom) {
    std::wstringstream sb;
    Cons cons = storage.getCons(atom);
    sb << "(" << toString(cons->car);
    if (isCons(cons->cdr) || isNil(cons->cdr)) {
        Atom val = cons->cdr;
        while (isCons(val)) {
            cons = storage.getCons(val);
            sb << " " << toString(cons->car);
            val = cons->cdr;
        }
        if (!isCons(val) && !isNil(val)) {
            sb << " " << toString(val);
        }
    } else {
        sb << "." << toString(cons->cdr);
    }
    sb << ")";
    return sb.str();
}

String Engine::toString(Atom atom) {
    if (isNil(atom)) {
        return std::wstring(L"NIL");
    }
    Word type = getType(atom);
    std::wstringstream sb;
    switch(type) {
    case TAG_TYPE_NUMBER:
        sb << getNumber(atom);
        return sb.str();
    case TAG_TYPE_BIF:
        return  std::wstring(L"$") + getBIFName(atom);
    case TAG_TYPE_GLOBAL:
        return  std::wstring(L"@") + storage.getGlobalName(atom);
    case TAG_TYPE_STRING:
        return std::wstring(L"'") + storage.getString(atom) + std::wstring(L"'");
    case TAG_TYPE_SYMBOL:
        return std::wstring(L"#") + storage.getSymbolName(atom);
    case TAG_TYPE_CONS:
        return printList(atom);
    default:
        return std::wstring(L"UNKNOWN");
    }
}

String Engine::toSimpleString(Atom atom) {
    if (isNil(atom)) {
        return std::wstring(L"");
    }
    Word type = getType(atom);
    std::wstringstream sb;
    switch(type) {
    case TAG_TYPE_NUMBER:
        sb << getNumber(atom);
        return sb.str();
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
    default:
        return std::wstring(L"");
    }
}
