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
#include "bytecodeparser.h"

#include <cassert>

void Engine::push(Atom& reg, Atom atom) {
   reg = storage.makeCons(atom, reg);
}

Atom Engine::pop(Atom& reg) {
    if (isNil(reg)) {
        return NIL;
    }
    Cons cons = storage.getCons(reg);
    Atom result = cons->car;
    reg = cons->cdr;
    return result;
}

Engine::Engine()
{
    initializeBIF();
}

Engine::~Engine() {

}

Atom Engine::makeBuiltInFunction(Atom nameSymbol, BIF value) {
    assert(isSymbol(nameSymbol));
    Word result = bifTable.add(nameSymbol, value);
    assert(result < MAX_INDEX_SIZE);
    return tagIndex(result, TAG_TYPE_BIF);
}

Atom Engine::findBuiltInFunction(Atom nameSymbol) {
    assert(isSymbol(nameSymbol));
    Word index;
    if (!bifTable.find(nameSymbol, index)) {
        return NIL;
    }
    return tagIndex(index, TAG_TYPE_BIF);
}

BIF Engine::getBuiltInFunction(Atom atom) {
    assert(isBIF(atom));
    return bifTable.getValue(untagIndex(atom));
}

String Engine::getBIFName(Atom atom) {
    assert(isBIF(atom));
    return storage.getSymbolName(bifTable.getKey(untagIndex(atom)));
}


Atom Engine::eval(String name, std::wistream& stream) {
    BytecodeParser p(stream, this);
    Atom at = p.parse();
    TRACE("Parsed: " << toString(at));
    return exec(name, at);
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
    push(s, storage.readGlobal(pop(c)));
}

void Engine::opSTG() {
    storage.writeGlobal(pop(c), pop(s));
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
    Cons funPair = storage.getCons(fun);
    push(d, c);
    push(d, e);
    push(d, s);
    s = NIL;
    c = funPair->car;
    e = storage.makeCons(v, funPair->cdr);
    push(p, storage.makeCons(currentFile, makeNumber(currentLine)));
    std::wcout << toString(p) << std::endl;
}

void Engine::opBAP() {
    Atom fun = pop(c);
    assert(isBIF(fun));
    BIF bif = getBuiltInFunction(fun);
    push(s, bif(this, storage, pop(s)));
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
    Cons cons = storage.getCons(pop(s));
    push(s, cons->car);
}

void Engine::opCDR() {
    Cons cons = storage.getCons(pop(s));
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
    Cons c = storage.getCons(cell);
    c->car = element;
    push(s, cell);
}

void Engine::opRPLACDR() {
    Atom element = pop(s);
    Atom cell = pop(s);
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

void Engine::opADD(int a, int b) {
    push(s, makeNumber(a + b));
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
    int b = getNumber(pop(s));
    int a = getNumber(pop(s));
    switch(opcode) {
    case SYMBOL_OP_ADD:
        opADD(a, b);
        return;
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
    Cons cons = storage.getCons(pos);
    Word i = getNumber(cons->car);
    Word j = getNumber(cons->cdr);
    Atom env = e;
    while (i > 1) {
        if (!isCons(env)) {
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
    Cons cons = storage.getCons(pos);
    Word i = getNumber(cons->car);
    Word j = getNumber(cons->cdr);
    Atom env = e;
    while (i > 1) {
        if (!isCons(env)) {
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
    currentLine = getNumber(pop(c));
}

void Engine::opFile() {
    currentFile = pop(c);
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
    case SYMBOL_OP_BAP:
        opBAP();
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
    }
}

Atom Engine::exec(String name, Atom code) {
    s = NIL;
    e = NIL;
    c = code;
    d = NIL;
    currentFile = storage.makeSymbol(name);
    currentLine = makeNumber(1);
    p = storage.makeCons(currentFile, currentLine);

    while (true) {
        Atom op = pop(c);
        if (isNil(op)) {
            return pop(s);
        }
        if (op == SYMBOL_OP_STOP) {
            return pop(s);
        }
        dispatch(op);
        if (shouldGC()) {
            gc();
        }
    }    
    gc();
    return s;
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
        if (!isCons(val)) {
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
