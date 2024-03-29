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
#include <deque>

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
    declaredFixedSymbol(SYMBOL_OP_NOOP, "NOOP");
    declaredFixedSymbol(SYMBOL_OP_CHAIN, "CHAIN");
    declaredFixedSymbol(SYMBOL_OP_CHAIN_END, "CHAINEND");
    declaredFixedSymbol(SYMBOL_OP_FILE, "FILE");
    declaredFixedSymbol(SYMBOL_OP_LINE, "LINE");
    declaredFixedSymbol(SYMBOL_OP_RPLCAR, "RPLCAR");
    declaredFixedSymbol(SYMBOL_OP_RPLCDR, "RPLCDR");
    declaredFixedSymbol(SYMBOL_VALUE_HOME_PATH, "HOME_PATH");
    declaredFixedSymbol(SYMBOL_VALUE_OP_COUNT, "OP_COUNT");
    declaredFixedSymbol(SYMBOL_VALUE_GC_COUNT, "GC_COUNT");
    declaredFixedSymbol(SYMBOL_VALUE_GC_EFFICIENCY,
                        "GC_EFFICIENCY");
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
    declaredFixedSymbol(SYMBOL_VALUE_NUM_REFERENCES_USED,
                        "NUM_REFERENCES_USED");
    declaredFixedSymbol(SYMBOL_VALUE_NUM_TOTAL_ARRAYS,
                        "NUM_TOTAL_ARRAYS");
    declaredFixedSymbol(SYMBOL_VALUE_NUM_ARRAYS_USED,
                        "NUM_ARRAYS_USED");
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
            // Every 10th GC is always a major (full) GC (we need to free
            // our value tables (strings table, large number table etc.)
            if (gcCounter % TUNING_PARAM_MAX_MINOR_GCS == 0 ) {
                FINE(log, "Starting MAJOR garbage collection...");
                gc(true, car, cdr);
            } else {
                // Try a minor GC first...
                FINE(log, "Starting MINOR garbage collection...");
                gc(false, car, cdr);
                if (cellSize - cellsInUse < TUNING_PARAM_MIN_FREE_SPACE) {
                    // Still not enough, run a full GC!
                    FINE(log, "Starting MAJOR garbage collection...");
                    gc(true, car, cdr);
                }
            }
        }
        if (cellSize - cellsInUse < TUNING_PARAM_MIN_FREE_SPACE) {
            Word oldSize = cellSize;
            cellSize += TUNING_PARAM_STORAGE_CHUNK_SIZE;
            FINE(log, "Expanding heap from: " <<
                 oldSize <<
                 " to: " <<
                 cellSize);
            if (cells == NULL) {
                cells = (Cell*)malloc(sizeof(Cell) * cellSize);
                states = (EntryState*)malloc(sizeof(EntryState) * cellSize);
            } else {
                cells = (Cell*)realloc(cells, sizeof(Cell) * cellSize);
                states = (EntryState*)realloc(states,
                                              sizeof(EntryState) * cellSize);
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

void Storage::gc(bool major, Atom car, Atom cdr) {

    if (major) {
        // Cleanup ref counts
        stringTable.resetRefCount();
        largeNumberTable.resetRefCount();
        decimalNumberTable.resetRefCount();
        referenceTable.resetRefCount();
        arrayTable.resetRefCount();
        for(Word i = 0; i < arrayTable.size(); i++)  {
            if (arrayTable.inUse(i)) {
                arrayTable.get(i).data()->checked = false;
            }
        }

        for(Word i = 0; i < cellSize; i++) {
            states[i] = GRAY;
        }
    }

    int gcRoots = 0;

    // Mark temporary variables
    if (isCons(car)) {
        states[untagIndex(car)] = REFERENCED;
        gcRoots++;
    } else {
        incValueTable(car, untagIndex(car), NULL);
    }
    if (isCons(cdr)) {
        states[untagIndex(cdr)] = REFERENCED;
        gcRoots++;
    } else {
        incValueTable(cdr, untagIndex(cdr), NULL);
    }

    // Mark globals as referenced
    for(Word i = 0; i < globalsTable.size(); i++) {
        if (isCons(globalsTable.getValue(i))) {
            states[untagIndex(globalsTable.getValue(i))] = REFERENCED;
            gcRoots++;
        } else {
            incValueTable(globalsTable.getValue(i),
                          untagIndex(globalsTable.getValue(i)), NULL);
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
            incValueTable(ref->atom(), untagIndex(ref->atom()), NULL);
        }
    }

    FINE(log, "GC: GC-Roots:" << gcRoots);

    // execute mark-phase
    mark();

    // execute sweep-phase
    sweep();

    if (major) {
        stringTable.gc();
        largeNumberTable.gc();
        decimalNumberTable.gc();
        referenceTable.gc();
        arrayTable.gc();
    }

    gcCounter++;
}

void Storage::incValueTable(Atom atom, Word idx, std::deque<Word>* refQueue) {
    if (isLargeNumber(atom)) {
        largeNumberTable.inc(idx);
    } else if (isDecimalNumber(atom)) {
        decimalNumberTable.inc(idx);
    } else if (isString(atom)) {
        stringTable.inc(idx);
    } else if (isArray(atom)) {
        arrayTable.inc(idx);
        Array* array = arrayTable.get(idx).data();
        if (!array->checked) {
            array->checked = true;
            for(int i = 1; i < array->length(); i++) {
                Atom a = array->at(i);
                Word aIdx = untagIndex(a);
                if (isCons(a)) {
                    if (states[aIdx] != CHECKED) {
                        states[aIdx] = REFERENCED;
                        if (refQueue != NULL) {
                            refQueue->push_back(aIdx);
                        }
                    }
                } else {
                    incValueTable(a, aIdx, refQueue);
                }
            }
        }
    } else if (isReference(atom)) {
        referenceTable.inc(idx);
    }
}

void Storage::markCell(Word index,
                       std::deque<Word>& refQueue,
                       bool alwaysQueue) {
    states[index] = CHECKED;
    Cell cell = cells[index];
    Word carIdx = untagIndex(cell.car);
    Word cdrIdx = untagIndex(cell.cdr);
    if (isCons(cell.car)) {
        if (states[carIdx] != CHECKED) {
            states[carIdx] = REFERENCED;
            if (alwaysQueue || carIdx < index) {
                refQueue.push_back(carIdx);
            }
        }
    } else {
        incValueTable(cell.car, carIdx, &refQueue);
    }
    if (isCons(cell.cdr)) {
        if (states[cdrIdx] != CHECKED) {
            states[cdrIdx] = REFERENCED;
            if (alwaysQueue || cdrIdx < index) {
                refQueue.push_back(cdrIdx);
            }
        }
    } else {
        incValueTable(cell.cdr, cdrIdx, &refQueue);
    }
}

void Storage::mark() {  
    std::deque<Word> refQueue;
    Word iterations = cellSize;
    Word used = 0;
    for(Word index = 0; index < cellSize; index++) {
        if (states[index] == REFERENCED) {
            used++;
            markCell(index, refQueue, false);
        }
    }
    while(!refQueue.empty()) {
        iterations++;
        Word index = refQueue.front();
        refQueue.pop_front();
        if (states[index] == REFERENCED) {
            used++;
            markCell(index, refQueue, true);
        }
    }

    FINE(log, "MARK: Iterations: " <<
         iterations <<
         ", Cells: " <<
         cellSize <<
         ", Marked: " << used);
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
        }
    }

    double eff =  (100.0 * reclaimed / cellSize);
    avgGCEfficiency.addValue(eff);

    FINE(log, "SWEEP: Reclaimed: " <<
         reclaimed << "(" << eff << "%" <<
         ", Avg: " << avgGCEfficiency.average() << "%)");
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

void Storage::setCAR(Atom atom, Atom car) {
    assert(isCons(atom));
    Word index = untagIndex(atom);

    if (states[index] == UNUSED) {
        ERROR(log, "Atom " <<
              atom <<
              " (Index: " <<
              index <<
              ") cannot be accessed!");
    }
    cells[index].car = car;
    // Due to a minor GC, the cell might be considered CHECKED. Since its
    // components changed, we must force the collector to check this cell
    // again.
    if (states[index] == CHECKED) {
        states[index] = REFERENCED;
    }
}

void Storage::setCDR(Atom atom, Atom cdr) {
    assert(isCons(atom));
    Word index = untagIndex(atom);

    if (states[index] == UNUSED) {
        ERROR(log, "Atom " <<
              atom <<
              " (Index: " <<
              index <<
              ") cannot be accessed!");
    }
    cells[index].cdr = cdr;
    // Due to a minor GC, the cell might be considered CHECKED. Since its
    // components changed, we must force the collector to check this cell
    // again.
    if (states[index] == CHECKED) {
        states[index] = REFERENCED;
    }
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

Array* Storage::getArray(Atom atom) {
    assert(isArray(atom));
    Word index = untagIndex(atom);
    return arrayTable.get(index).data();
}

Atom Storage::makeArray(int size) {
    Array* result = new Array(size);
    Word index = arrayTable.allocate(QSharedPointer<Array>(result));
    assert(index < MAX_INDEX_SIZE);
    return tagIndex(index, TAG_TYPE_ARRAY);
}

Reference* Storage::getReference(Atom atom) {
    assert(isReference(atom));
    Word index = untagIndex(atom);
    return referenceTable.get(index).data();
}

Atom Storage::makeReference(Reference* value) {
    Word index = referenceTable.allocate(QSharedPointer<Reference>(value));
    assert(index < MAX_INDEX_SIZE);
    return tagIndex(index, TAG_TYPE_REFERENCE);
}

QString Storage::getString(Atom atom) {
    assert(isString(atom));
    Word index = untagIndex(atom);
    return stringTable.get(index);
}

Atom Storage::makeNumber(Number value) {
    if ((value & LOST_BITS) == 0 || (value & LOST_BITS) == LOST_BITS) {
        return static_cast<Word>(value) << TAG_LENGTH | TAG_TYPE_NUMBER;
    } else {
        Word index = largeNumberTable.allocate(value);
        assert(index < MAX_INDEX_SIZE);
        return tagIndex(index, TAG_TYPE_LARGE_NUMBER);
    }
}

Number Storage::getNumber(Atom atom) {
    assert(isNumber(atom));
    if (isSmallNumber(atom)) {
        Word result = atom >> TAG_LENGTH;
        if (atom & SIGN_CHECK_BIT) {
            result |= LOST_BITS;
        }
        return static_cast<Number>(result);
    } else {
        Word index = untagIndex(atom);
        return largeNumberTable.get(index);
    }
}

