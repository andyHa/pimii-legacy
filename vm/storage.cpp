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

#include "storage.h"

#include <iomanip>
#include <algorithm>

Storage::Storage() {
    initializeSymbols();
}

void Storage::declaredFixedSymbol(Word expected, const char* name) {
    Atom value = makeSymbol(name);
    assert(value == expected);
}

void Storage::initializeSymbols() {
    declaredFixedSymbol(SYMBOL_TRUE, "TRUE");
    declaredFixedSymbol(SYMBOL_FALSE, "FALSE");
    declaredFixedSymbol(SYMBOL_TYPE_CONS, "TYPE_CONS");
    declaredFixedSymbol(SYMBOL_TYPE_NUMBER, "TYPE_NUMBER");
    declaredFixedSymbol(SYMBOL_TYPE_DECIMAL, "TYPE_DECIMAL");
    declaredFixedSymbol(SYMBOL_TYPE_STRING, "TYPE_STRING");
    declaredFixedSymbol(SYMBOL_TYPE_REFERENCE, "TYPE_REFERENCE");
    declaredFixedSymbol(SYMBOL_TYPE_SYMBOL, "TYPE_SYMBOL");
    declaredFixedSymbol(SYMBOL_TYPE_BIF, "TYPE_BIF");
    declaredFixedSymbol(SYMBOL_TYPE_GLOBAL, "TYPE_GLOBAL");

    declaredFixedSymbol(SYMBOL_TAG_CLOSE_END, " />");
    declaredFixedSymbol(SYMBOL_TAG_END, ">");
    declaredFixedSymbol(SYMBOL_TAG_QUOTE, "\"");

    declaredFixedSymbol(SYMBOL_OP_NIL, "NIL");
    declaredFixedSymbol(SYMBOL_OP_LD, "LD");
    declaredFixedSymbol(SYMBOL_OP_LDC, "LDC");
    declaredFixedSymbol(SYMBOL_OP_LDF, "LDF");
    declaredFixedSymbol(SYMBOL_OP_AP, "AP");
    declaredFixedSymbol(SYMBOL_OP_RTN, "RTN");
    declaredFixedSymbol(SYMBOL_OP_BT, "BT");
    declaredFixedSymbol(SYMBOL_OP_AP0, "AP0");
    declaredFixedSymbol(SYMBOL_OP_ST, "ST");
    declaredFixedSymbol(SYMBOL_OP_LDG, "LDG");
    declaredFixedSymbol(SYMBOL_OP_STG, "STG");
    declaredFixedSymbol(SYMBOL_OP_CAR, "CAR");
    declaredFixedSymbol(SYMBOL_OP_CDR, "CDR");
    declaredFixedSymbol(SYMBOL_OP_CONS, "CONS");
    declaredFixedSymbol(SYMBOL_OP_EQ, "EQ");
    declaredFixedSymbol(SYMBOL_OP_NE, "NE");
    declaredFixedSymbol(SYMBOL_OP_LT, "LT");
    declaredFixedSymbol(SYMBOL_OP_GT, "GT");
    declaredFixedSymbol(SYMBOL_OP_LTQ, "LTQ");
    declaredFixedSymbol(SYMBOL_OP_GTQ, "GTQ");
    declaredFixedSymbol(SYMBOL_OP_ADD, "ADD");
    declaredFixedSymbol(SYMBOL_OP_SUB, "SUB");
    declaredFixedSymbol(SYMBOL_OP_MUL, "MUL");
    declaredFixedSymbol(SYMBOL_OP_DIV, "DIV");
    declaredFixedSymbol(SYMBOL_OP_REM, "REM");
    declaredFixedSymbol(SYMBOL_OP_NOT, "NOT");
    declaredFixedSymbol(SYMBOL_OP_AND, "AND");
    declaredFixedSymbol(SYMBOL_OP_OR, "OR");
    declaredFixedSymbol(SYMBOL_OP_STOP, "STOP");
    declaredFixedSymbol(SYMBOL_OP_SPLIT, "SPLIT");
    declaredFixedSymbol(SYMBOL_OP_CONCAT, "CONCAT");
    declaredFixedSymbol(SYMBOL_OP_SCAT, "SCAT");
    declaredFixedSymbol(SYMBOL_OP_CHAIN, "CHAIN");
    declaredFixedSymbol(SYMBOL_OP_CHAIN_END, "CHAINEND");
    declaredFixedSymbol(SYMBOL_OP_FILE, "FILE");
    declaredFixedSymbol(SYMBOL_OP_LINE, "LINE");
}


Atom Storage::makeSymbol(const QString& name) {
    Word result = symbolTable.add(name, name);
    assert(result < MAX_INDEX_SIZE);
    return tagIndex(result, TAG_TYPE_SYMBOL);
}

QString Storage::getSymbolName(Atom symbol) {
    assert(isSymbol(symbol));
    return symbolTable.getKey(untagIndex(symbol));
}

std::pair<Atom, Cons> Storage::cons(Atom car, Atom cdr) {


    if (!freeList.empty()) {
        Word index = freeList.back();
        freeList.pop_back();
        Cons cons = cells[index].cell;
        cons->car = car;
        cons->cdr = cdr;
        std::pair<Atom, Cons> pair(tagIndex(index, TAG_TYPE_CONS), cons);
        return pair;
    } else {
        Cons cons = new Cell();
        cons->car = car;
        cons->cdr = cdr;

        StorageEntry entry;
        entry.cell = cons;
        entry.state = UNUSED;
        Word result = cells.size();
        assert(result < MAX_INDEX_SIZE);
        cells.push_back(entry);
        std::pair<Atom, Cons> pair(tagIndex(result, TAG_TYPE_CONS), cons);
        return pair;
    }
}

Atom Storage::makeCons(Atom car, Atom cdr) {
    return cons(car, cdr).first;
}

Cons Storage::getCons(Atom atom) {
    assert(isCons(atom));
    return cells[untagIndex(atom)].cell;
}

StorageStatus Storage::getStatus() {
    StorageStatus status;
    status.cellsUsed = (Word)cells.size() - (Word)freeList.size();
    status.totalCells = cells.size();
    status.numSymbols = symbolTable.size();
    status.numGlobals = globalsTable.size();
    status.stringsUsed = stringTable.getNumberOfUsedCells();
    status.totalStrings = stringTable.getTotalCells();
    status.numbersUsed = largeNumberTable.getNumberOfUsedCells();
    status.totalNumbers = largeNumberTable.getNumberOfUsedCells();
    status.deicmalsUsed = decimalNumberTable.getNumberOfUsedCells();
    status.totalDecimals = decimalNumberTable.getNumberOfUsedCells();
    return status;
}

void Storage::beginGC() {
    for(Word i = 0; i < globalsTable.size(); i++) {
        if (isCons(globalsTable.getValue(i))) {
            cells[untagIndex(globalsTable.getValue(i))].state = REFERENCED;
        }
    }
    stringTable.resetRefCount();
    largeNumberTable.resetRefCount();
    decimalNumberTable.resetRefCount();
    referenceTable.resetRefCount();
}

void Storage::markGCRoot(Atom atom) {
    if (isCons(atom)) {
        cells[untagIndex(atom)].state = REFERENCED;
    }
}

void Storage::incValueTable(Atom atom, Word idx) {
    if (isLargeNumber(atom)) {
        largeNumberTable.inc(idx);
    } else if (isDecimalNumber(atom)) {
        decimalNumberTable.inc(idx);
    } else if (isString(atom)) {
        stringTable.inc(idx);
    } else if (isReference(atom)) {
        referenceTable.inc(idx);
    }
}

void Storage::mark() {  
    Word index = 0;
    while(index < cells.size()) {
        if (cells[index].state == REFERENCED) {
            cells[index].state = CHECKED;
            Cons cell = cells[index].cell;
            Word carIdx = untagIndex(cell->car);
            Word cdrIdx = untagIndex(cell->cdr);
            index++;
            if (isCons(cell->car) && cells[carIdx].state != CHECKED) {
                 cells[carIdx].state = REFERENCED;
                 if (carIdx < index) {
                     index = carIdx;
                 }
            } else {
                incValueTable(cell->car, carIdx);
            }
            if (isCons(cell->cdr) && cells[cdrIdx].state != CHECKED) {
                 cells[cdrIdx].state = REFERENCED;
                 if (cdrIdx < index) {
                     index = cdrIdx;
                 }
            } else {
                incValueTable(cell->cdr, cdrIdx);
            }
        } else {
            index++;
        }
    }
}

void Storage::sweep() {
    freeList.clear();
    for(Word i = 0; i < cells.size(); i++) {
        if (cells[i].state == UNUSED) {
            freeList.push_back(i);
        } else {
            cells[i].state = UNUSED;
        }
    }
    stringTable.gc();
    largeNumberTable.gc();
    decimalNumberTable.gc();
    referenceTable.gc();
}

Atom Storage::cons(Cons cons, Atom next) {
    Atom tmp = makeCons(next, NIL);
    cons->cdr = tmp;
    return tmp;
}

Atom Storage::findGlobal(Atom nameSymbol) {
    assert(isSymbol(nameSymbol));
    Word result = globalsTable.add(nameSymbol, NIL);
    assert(result < MAX_INDEX_SIZE);
    return tagIndex(result, TAG_TYPE_GLOBAL);
}

QString Storage::getGlobalName(Atom atom) {
    assert(isGlobal(atom));
    return getSymbolName(globalsTable.getKey(untagIndex(atom)));
}

Atom Storage::readGlobal(Atom atom) {
    assert(isGlobal(atom));
    return globalsTable.getValue(untagIndex(atom));
}

void Storage::writeGlobal(Atom atom, Atom value) {
    assert(isGlobal(atom));
    globalsTable.setValue(untagIndex(atom), value);
}

Atom Storage::makeString(const QString& string) {
    Word index = stringTable.allocate(string);
    assert(index < MAX_INDEX_SIZE);
    return tagIndex(index, TAG_TYPE_STRING);
}

double Storage::getDecimal(Atom atom) {
    assert(isDecimalNumber(atom));
    Word index = untagIndex(atom);
    return decimalNumberTable.get(index);
}
Atom Storage::makeDecimal(double value) {
    Word index = decimalNumberTable.allocate(value);
    assert(index < MAX_INDEX_SIZE);
    return tagIndex(index, TAG_TYPE_DECIMAL_NUMBER);
}

QSharedPointer<Reference> Storage::getReference(Atom atom) {
    assert(isReference(atom));
    Word index = untagIndex(atom);
    return referenceTable.get(index);
}
Atom Storage::makeReference(const QSharedPointer<Reference>& value) {
    Word index = referenceTable.allocate(value);
    assert(index < MAX_INDEX_SIZE);
    return tagIndex(index, TAG_TYPE_REFERENCE);
}

QString Storage::getString(Atom atom) {
    assert(isString(atom));
    Word index = untagIndex(atom);
    return stringTable.get(index);
}

Atom Storage::makeNumber(long value) {
    if ((value & LOST_BITS) == 0 || (value & LOST_BITS) == LOST_BITS) {
        return value << TAG_LENGTH | TAG_TYPE_NUMBER;
    } else {
        Word index = largeNumberTable.allocate(value);
        assert(index < MAX_INDEX_SIZE);
        return tagIndex(index, TAG_TYPE_LARGE_NUMBER);
    }
}

long Storage::getNumber(Atom atom) {
    assert(isNumber(atom) || isLargeNumber(atom));
    if (isNumber(atom)) {
        Word result = atom >> TAG_LENGTH;
        if (atom & SIGN_CHECK_BIT) {
            result |= LOST_BITS;
        }
        return (long)result;
    } else {
        Word index = untagIndex(atom);
        return largeNumberTable.get(index);
    }
}


