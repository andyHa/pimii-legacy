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

Storage::Storage() : log("STORE") {
    initializeSymbols();
    gcCounter = 0;
    nextFree = 0;
    cellSize = 0;
    cellsInUse = 0;
    cells = NULL;
    states = NULL;
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
    declaredFixedSymbol(SYMBOL_VALUE_OP_CODES_PER_EVENT_LOOP,
                        "OP_CODES_PER_EVENT_LOOP");
    declaredFixedSymbol(SYMBOL_VALUE_HOME_PATH, "HOME_PATH");
    declaredFixedSymbol(SYMBOL_VALUE_GC_MIN_CELLS, "GC_MIN_CELLS");
    declaredFixedSymbol(SYMBOL_VALUE_DEBUG_COMPILER, "DEBUG_COMPILER");
    declaredFixedSymbol(SYMBOL_VALUE_DEBUG_ENGINE, "DEBUG_ENGINE");
    declaredFixedSymbol(SYMBOL_VALUE_DEBUG_STORAGE, "DEBUG_STORAGE");
    declaredFixedSymbol(SYMBOL_VALUE_OP_COUNT, "OP_COUNT");
    declaredFixedSymbol(SYMBOL_VALUE_GC_COUNT, "GC_COUNT");
    declaredFixedSymbol(SYMBOL_VALUE_NUM_GC_ROOTS, "NUM_GC_ROOTS");
    declaredFixedSymbol(SYMBOL_VALUE_NUM_SYMBOLS, "NUM_SYMBOLS");
    declaredFixedSymbol(SYMBOL_VALUE_NUM_GLOBALS, "NUM_GLOBALS");
    declaredFixedSymbol(SYMBOL_VALUE_NUM_TOTAL_CELLS, "NUM_TOTAL_CELLS");
    declaredFixedSymbol(SYMBOL_VALUE_NUM_CELLS_USED, "NUM_CELLS_USED");
    declaredFixedSymbol(SYMBOL_VALUE_NUM_TOTAL_STRINGS, "NUM_TOTAL_STRINGS");
    declaredFixedSymbol(SYMBOL_VALUE_NUM_STRINGS_USED, "NUM_STRINGS_USED");
    declaredFixedSymbol(SYMBOL_VALUE_NUM_TOTAL_NUMBERS, "NUM_TOTAL_NUMBERS");
    declaredFixedSymbol(SYMBOL_VALUE_NUM_NUMBERS_USED, "NUM_NUMBERS_USED");
    declaredFixedSymbol(SYMBOL_VALUE_NUM_TOTAL_DECIMALS, "NUM_TOTAL_DECIMALS");
    declaredFixedSymbol(SYMBOL_VALUE_NUM_DECIMALS_USED, "NUM_DECIMALS_USED");
    declaredFixedSymbol(SYMBOL_VALUE_NUM_TOTAL_REFERENCES,
                        "NUM_TOTAL_REFERENCES");
    declaredFixedSymbol(SYMBOL_VALUE_NUM_REFERENES_USED,
                        "SYMBOL_VALUE_NUM_REFERENES_USED");

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

Atom Storage::makeCons(Atom car, Atom cdr) {
    if (nextFree >= cellSize) {
        TRACE(log, "Memory maxed out...");
        if (cellSize > 0) {
            FINE(log, "Starting garbage collection...");
            gc(car, cdr);
        }
        if (cellSize - cellsInUse < 512) {
            Word oldSize = cellSize;
            cellSize += 4096;
            FINE(log, "Expanding heap from: " << oldSize << " to: " << cellSize);
            if (cells == NULL) {
                cells = (Cell*)malloc(sizeof(Cell) * cellSize);
                states = (EntryState*)malloc(sizeof(EntryState) * cellSize);
            } else {
                cells = (Cell*)realloc(cells, sizeof(Cell) * cellSize);
                states = (EntryState*)realloc(states, sizeof(EntryState) * cellSize);
            }
            for(Word i = cellSize; i > oldSize ; i--) {
                cells[i - 1].car = i;
                states[i - 1] = UNUSED;
            }
            nextFree = oldSize;
        }
    }

    Word index = nextFree;
    assert(states[index] == UNUSED);
    nextFree = cells[index].car;
    cells[index].car = car;
    cells[index].cdr = cdr;
    states[index] = GRAY;
    cellsInUse++;
    assert(index < MAX_INDEX_SIZE);
    return tagIndex(index, TAG_TYPE_CONS);
}

void Storage::gc(Atom car, Atom cdr) {
    // Cleanup ref counts
    stringTable.resetRefCount();
    largeNumberTable.resetRefCount();
    decimalNumberTable.resetRefCount();
    referenceTable.resetRefCount();

    int gcRoots = 0;

    // Mark temporary variables
    if (isCons(car)) {
        states[untagIndex(car)] = REFERENCED;
        gcRoots++;
    } else {
        incValueTable(car, untagIndex(car));
    }
    if (isCons(cdr)) {
        states[untagIndex(cdr)] = REFERENCED;
        gcRoots++;
    } else {
        incValueTable(cdr, untagIndex(cdr));
    }

    // Mark globals as referenced
    for(Word i = 0; i < globalsTable.size(); i++) {
        if (isCons(globalsTable.getValue(i))) {
            states[untagIndex(globalsTable.getValue(i))] = REFERENCED;
            gcRoots++;
        } else {
            incValueTable(globalsTable.getValue(i),
                          untagIndex(globalsTable.getValue(i)));
        }
    }

    // Mark strong references as referenced
    for(std::set<AtomRef*>::iterator
        iter = strongReferences.begin();
        iter != strongReferences.end();
        ++iter) {
        AtomRef* ref = *iter;
        if (isCons(ref->atom())) {
            states[untagIndex(ref->atom())] = REFERENCED;
            gcRoots++;
        } else {
            incValueTable(ref->atom(), untagIndex(ref->atom()));
        }
    }

    FINE(log, "GC: GC-Roots:" << gcRoots);

    // execute mark-phase
    mark();

    // execute sweep-phase
    sweep();

    gcCounter++;
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
    Word iterations = 0;
    Word used = 0;
    while(index < cellSize) {
        iterations++;
        if (states[index] == REFERENCED) {
            used++;
            states[index] = CHECKED;
            Cell cell = cells[index];
            Word carIdx = untagIndex(cell.car);
            Word cdrIdx = untagIndex(cell.cdr);
            index++;
            if (isCons(cell.car) && states[carIdx] != CHECKED) {
                 states[carIdx] = REFERENCED;
                 if (carIdx < index) {
                     index = carIdx;
                 }
            } else {
                incValueTable(cell.car, carIdx);
            }
            if (isCons(cell.cdr) && states[cdrIdx] != CHECKED) {
                 states[cdrIdx] = REFERENCED;
                 if (cdrIdx < index) {
                     index = cdrIdx;
                 }
            } else {
                incValueTable(cell.cdr, cdrIdx);
            }
        } else {
            index++;
        }
    }
    FINE(log, "MARK: Iterations: " <<
         iterations <<
         ", Cells: " <<
         cellSize <<
         ", Used: " << used);
}

void Storage::sweep() {
    nextFree = cellSize;
    Word reclaimed = 0;
    for(Word i = 0; i < cellSize; i++) {
        if (states[i] == UNUSED || states[i] == GRAY) {
            cellsInUse--;
            states[i] = UNUSED;
            reclaimed++;
            cells[i].car = nextFree;
            nextFree = i;
        } else {
            states[i] = GRAY;
        }
    }

    FINE(log, "SWEEP: Reclaimed: " <<
         reclaimed <<
         "(" <<
         (100.0 * reclaimed / cellSize) <<
         "%)");

    stringTable.gc();
    largeNumberTable.gc();
    decimalNumberTable.gc();
    referenceTable.gc();
}

AtomRef* Storage::ref(Atom atom) {
    AtomRef* result = new AtomRef(this, atom);

    return result;
}

Atom Storage::append(Atom tail, Atom next) {
    AtomRef tailRef(this, tail);
    Atom tmp = makeCons(next, NIL);
    setCDR(tailRef.atom(), tmp);
    return tmp;
}

Cell Storage::getCons(Atom atom) {
    assert(isCons(atom));
    Word index = untagIndex(atom);
    if (states[index] != GRAY) {
        TRACE(log, "ACCESS: " << atom);
        assert(states[index] == GRAY);
    }
    return cells[index];
}

void Storage::setCAR(Atom atom, Atom car) {
    assert(isCons(atom));
    Word index = untagIndex(atom);
    assert(states[index] == GRAY);
    cells[index].car = car;
}

void Storage::setCDR(Atom atom, Atom cdr) {
    assert(isCons(atom));
    Word index = untagIndex(atom);
    assert(states[index] == GRAY);
    cells[index].cdr = cdr;
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
    assert(isNumber(atom));
    if (isSmallNumber(atom)) {
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

