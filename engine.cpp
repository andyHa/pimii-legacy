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


Atom Engine::eval(std::wistream& stream) {
    BytecodeParser p(stream, this);
    Atom at = p.parse();
    TRACE("Parsed: " << toString(at));
    return exec(at);
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
    push(s, storage.makeCons(pop(s), e));
}

void Engine::opAP() {
    Atom fun = pop(s);
    Cons funPair = storage.getCons(fun);
    Atom v = pop(s);
    push(d, c);
    push(d, e);
    push(d, s);
    s = NIL;
    c = funPair->car;
    e = storage.makeCons(v, funPair->cdr);
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
    Atom a = pop(s);
    Atom b = pop(s);
    push(s, storage.makeCons(a,b));
}

void Engine::opEQ(int a, int b) {
    push(s, a == b ? SYMBOL_TRUE : SYMBOL_FALSE);
}

void Engine::opADD(int a, int b) {
    push(s, makeNumber(a + b));
}

void Engine::dispatchArithmetic(Atom opcode) {
    int a = getNumber(pop(s));
    int b = getNumber(pop(s));
    switch(opcode) {
    case SYMBOL_OP_ADD:
        opADD(a, b);
        return;
    case SYMBOL_OP_EQ:
        opEQ(a, b);
        return;
    }
}

Atom Engine::locate(Atom pos) {
    Cons cons = storage.getCons(pos);
    Word i = getNumber(cons->car);
    Word j = getNumber(cons->cdr);
    Atom env = e;
    while (i > 1) {
        if (isNil(env)) {
            return NIL;
        }
        env = storage.getCons(env)->cdr;
        i--;
    }
    if (isNil(env)) {
        return NIL;
    }
    env = storage.getCons(env)->car;
    while (j > 1) {
        if (isNil(env)) {
            return NIL;
        }
        env = storage.getCons(env)->cdr;
        j--;
    }
    if (isNil(env)) {
        return NIL;
    }
    return storage.getCons(env)->car;
}

void Engine::store(Atom pos, Atom value) {
    /*
    long i = loc.car().cast(IntegerConstant.class).getValue();
    long j = loc.cdr().cast(IntegerConstant.class).getValue();
    Value env = e.getValue();
    if (env == Nil.NIL) {
            env = new Cons();
            e.setValue(env);
    }
    while (i > 1) {
            if (env.cdr() == Nil.NIL) {
                    ((Cons) env).cdr(new Cons());
            }
            env = env.cdr();
            i--;
    }
    env = env.car();
    if (env == Nil.NIL) {
            Cons c = new Cons();
            ((Cons) env).car(c);
            env = c;
    }
    while (j > 1) {
            if (env.cdr() == Nil.NIL) {
                    ((Cons) env).cdr(new Cons());
            }
            env = env.cdr();
            j--;
    }
    ((Cons) env).car(value);
    */
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
    case SYMBOL_OP_LE:
    case SYMBOL_OP_LEQ:
    case SYMBOL_OP_GT:
    case SYMBOL_OP_GTQ:
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
    }
}

Atom Engine::exec(Atom code) {
    s = NIL;
    e = NIL;
    c = code;
    d = NIL;
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
    if (isCons(cons->cdr)) {
        Atom val = cons->cdr;
        while (!isNil(val)) {
            cons = storage.getCons(val);
            sb << " " << toString(cons->car);
            val = cons->cdr;
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
