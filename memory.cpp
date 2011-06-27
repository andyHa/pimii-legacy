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
  see memory.h
  */
#include <map>
#include <vector>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <cassert>

#include <QAtomicInt>
#include <QMutex>

#include "memory.h"
#include "tools.h"
#include "lookuptable.h"
#include "valuetable.h"


namespace memory {


    // ----------------------------------------------------------------------------
    // Internally used functions
    // ----------------------------------------------------------------------------




    // ----------------------------------------------------------------------------
    // Global data structures used to manage the memory
    // ----------------------------------------------------------------------------

    /**
      Use locking-mechanism provided by QT.
      */
    typedef QMutexLocker Locker;



    /**
      Maps symbols to unique global indices
      */
    LookupTable <Word, Atom, Word, Locker> globalsTable;



    /**
      Contains the cell storage. NO GC YET!
      */
    std::vector <Cons> cells;

    /**
      Contains the table of internally used strings...
      */
    ValueTable <Word, std::wstring> stringTable;

    // ----------------------------------------------------------------------------
    // Memory management for the values table.
    // ----------------------------------------------------------------------------

    void decrementReferences(Atom value) {
        assert(isValue(value));
        Word idx = untagIndex(value);
        stringTable.decReferences(idx);
    }

    void incrementReferences(Atom value) {
        assert(isValue(value));
        Word idx = untagIndex(value);
        stringTable.incReferences(idx);
    }

    // ----------------------------------------------------------------------------
    // Implementation
    // ----------------------------------------------------------------------------

    void initialize() {
        symbolTable.clear();
        bifTable.clear();
        globalsTable.clear();

        cells.clear();
        stringTable.clear();

        declaredFixedSymbol(SYMBOL_TRUE, L"TRUE");
        declaredFixedSymbol(SYMBOL_FALSE, L"FALSE");
        declaredFixedSymbol(SYMBOL_OP_NIL, L"NIL");
        declaredFixedSymbol(SYMBOL_OP_LD, L"LD");
        declaredFixedSymbol(SYMBOL_OP_LDC, L"LDC");
        declaredFixedSymbol(SYMBOL_OP_LDF, L"LDF");
        declaredFixedSymbol(SYMBOL_OP_AP, L"AP");
        declaredFixedSymbol(SYMBOL_OP_RTN, L"RTN");
        declaredFixedSymbol(SYMBOL_OP_SEL, L"SEL");
        declaredFixedSymbol(SYMBOL_OP_JOIN, L"JOIN");
        declaredFixedSymbol(SYMBOL_OP_RAP, L"RAP");
        declaredFixedSymbol(SYMBOL_OP_DUM, L"DUM");
        declaredFixedSymbol(SYMBOL_OP_ST, L"ST");
        declaredFixedSymbol(SYMBOL_OP_LDG, L"LDG");
        declaredFixedSymbol(SYMBOL_OP_STG, L"STG");
        declaredFixedSymbol(SYMBOL_OP_BAP, L"BAP");
        declaredFixedSymbol(SYMBOL_OP_CAR, L"CAR");
        declaredFixedSymbol(SYMBOL_OP_CDR, L"CDR");
        declaredFixedSymbol(SYMBOL_OP_CONS, L"CONS");
        declaredFixedSymbol(SYMBOL_OP_EQ, L"EQ");
        declaredFixedSymbol(SYMBOL_OP_NE, L"NE");
        declaredFixedSymbol(SYMBOL_OP_LE, L"LE");
        declaredFixedSymbol(SYMBOL_OP_GT, L"GT");
        declaredFixedSymbol(SYMBOL_OP_LEQ, L"LEQ");
        declaredFixedSymbol(SYMBOL_OP_GTQ, L"GTQ");
        declaredFixedSymbol(SYMBOL_OP_ADD, L"ADD");
        declaredFixedSymbol(SYMBOL_OP_SUB, L"SUB");
        declaredFixedSymbol(SYMBOL_OP_MUL, L"MUL");
        declaredFixedSymbol(SYMBOL_OP_DIV, L"DIV");
        declaredFixedSymbol(SYMBOL_OP_REM, L"REM");
        declaredFixedSymbol(SYMBOL_OP_NOT, L"NOT");
        declaredFixedSymbol(SYMBOL_OP_AND, L"AND");
        declaredFixedSymbol(SYMBOL_OP_OR, L"OR");
        declaredFixedSymbol(SYMBOL_OP_STOP, L"STOP");

        initializeBIF();
    }

    Atom Cell::car() {
        return this->head;
    }

    Atom Cell::cdr() {
        return this->tail;
    }

    void Cell::car(Atom atom) {
        if (isValue(atom)) {
            incrementReferences(atom);
        }
        if (isValue(this->head)) {
            decrementReferences(this->head);
        }
        this->head = atom;
    }

    void Cell::cdr(Atom atom) {
        if (isValue(atom)) {
            incrementReferences(atom);
        }
        if (isValue(this->tail)) {
            decrementReferences(this->tail);
        }
        this->tail = atom;
    }

    bool isNil(Atom atom) {
        return atom == NIL;
    }

    bool isSymbol(Atom atom) {
        return getType(atom) == TAG_TYPE_SYMBOL;
    }

    bool isNumber(Atom atom) {
        return getType(atom) == TAG_TYPE_NUMBER;
    }

    bool isCons(Atom atom) {
        return getType(atom) == TAG_TYPE_CONS;
    }

    bool isBIF(Atom atom) {
        return getType(atom) == TAG_TYPE_BIF;
    }

    bool isGlobal(Atom atom) {
        return getType(atom) == TAG_TYPE_GLOBAL;
    }

    bool isValue(Atom atom) {
        return getType(atom) == TAG_TYPE_VALUE;
    }

    Word getType(Atom atom) {
        return atom & TAG_MASK;
    }

    Atom makeSymbol(std::wstring name) {
        Word result = symbolTable.add(name, name);
        assert(result < MAX_INDEX_SIZE);
        return tagIndex(result, TAG_TYPE_SYMBOL);
    }

    std::wstring getName(Atom symbol) {
        assert(isSymbol(symbol));
        return symbolTable.getKey(untagIndex(symbol));
    }

    Atom makeNumber(int value) {
        if ((value & LOST_BITS) == 0 || (value & LOST_BITS) == LOST_BITS) {
            return value << TAG_LENGTH | TAG_TYPE_NUMBER;
        } else {
            throw "Overflow error";
        }
    }

    int getNumber(Atom atom) {
        Word result = atom >> TAG_LENGTH;
        if (atom & SIGN_CHECK_BIT) {
            result |= LOST_BITS;
        }
        return (int)result;
    }

    Atom makeCons(Atom car, Atom cdr) {
        Cons cons = new Cell();
        cons->car(car);
        cons->cdr(cdr);
        Word result = cells.size();
        assert(result < MAX_INDEX_SIZE);
        cells.push_back(cons);
        return tagIndex(result, TAG_TYPE_CONS);
    }

    Atom cons(Cons cons, Atom next) {
        Atom tmp = makeCons(next, NIL);
        cons->cdr(tmp);
        return tmp;
    }

    Atom cons(Atom cons, Atom next) {
        Atom tmp = makeCons(next, memory::NIL);
        getCons(cons)->cdr(tmp);
        return tmp;
    }

    Cons getCons(Atom cons) {
        assert(isCons(cons));
        return cells[untagIndex(cons)];
    }

    Atom makeBuiltInFunction(Atom nameSymbol, BIF value) {
        assert(isSymbol(nameSymbol));
        Word result = bifTable.add(nameSymbol, value);
        assert(result < MAX_INDEX_SIZE);
        return tagIndex(result, TAG_TYPE_BIF);
    }

    Atom findBuiltInFunction(Atom nameSymbol) {
        assert(isSymbol(nameSymbol));
        Word index;
        if (!bifTable.find(nameSymbol, index)) {
            return NIL;
        }
        return tagIndex(index, TAG_TYPE_BIF);
    }

    BIF getBuiltInFunction(Atom atom) {
        assert(isBIF(atom));
        return bifTable.getValue(untagIndex(atom));
    }

    std::wstring getBIFName(Atom atom) {
        assert(isBIF(atom));
        return getName(bifTable.getKey(untagIndex(atom)));
    }

    Atom findGlobal(Atom nameSymbol) {
        assert(isSymbol(nameSymbol));
        Word result = globalsTable.add(nameSymbol, NIL);
        assert(result < MAX_INDEX_SIZE);
        return tagIndex(result, TAG_TYPE_GLOBAL);
    }

    std::wstring getGlobalName(Atom atom) {
        assert(isGlobal(atom));
        return getName(globalsTable.getKey(untagIndex(atom)));
    }

    Atom readGlobal(Atom atom) {
        assert(isGlobal(atom));
        return globalsTable.getValue(untagIndex(atom));
    }

    void writeGlobal(Atom atom, Atom value) {
        assert(isGlobal(atom));
        globalsTable.setValue(untagIndex(atom), value);
    }

    Atom makeString(std::wstring string) {
        Word index = stringTable.allocate(string);
        assert(index < MAX_INDEX_SIZE);
        return tagIndex(index, TAG_TYPE_VALUE);
    }

    std::wstring getString(Atom atom) {
        assert(isValue(atom));
        Word index = untagIndex(atom);
        return stringTable.get(index);
    }

    std::wstring printList(Atom atom) {
        std::wstringstream sb;
        Cons cons = getCons(atom);
        sb << "(" << toString(cons->car());
        if (isCons(cons->cdr())) {
            Atom val = cons->cdr();
            while (!isNil(val)) {
                cons = getCons(val);
                sb << " " << toString(cons->car());
                val = cons->cdr();
            }
        } else {
            sb << "." << toString(cons->cdr());
        }
        sb << ")";
        return sb.str();
    }

    std::wstring toString(Atom atom) {
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
            return  std::wstring(L"@") + getGlobalName(atom);
        case TAG_TYPE_VALUE:
            return std::wstring(L"'") + getString(atom) + std::wstring(L"'");
        case TAG_TYPE_SYMBOL:
            return std::wstring(L"#") + getName(atom);
        case TAG_TYPE_CONS:
            return printList(atom);
        default:
            return std::wstring(L"UNKNOWN");
        }
    }

    std::wstring toSimpleString(Atom atom) {
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
            return getGlobalName(atom);
        case TAG_TYPE_VALUE:
            return getString(atom);
        case TAG_TYPE_SYMBOL:
            return getName(atom);
        case TAG_TYPE_CONS:
            return printList(atom);
        default:
            return std::wstring(L"");
        }
    }

    // ----------------------------------------------------------------------------
    // Statistics and Debug infos...
    // ----------------------------------------------------------------------------

    /**
      Returns the number of cells allocated.
      */
    Word getNumberOfCells() {
        return cells.size();
    }

    /**
      Returns the total amount of memory used by the cell storage.
      */
    Word getCellsMemSize() {
        return getNumberOfCells() * CELL_SIZE;
    }

    /**
      Returns the number of values allocated.
      */
    Word getNumberOfValues() {
        return stringTable.size();
    }

    std::wstring toDumpString(Atom atom) {
        if (isNil(atom)) {
            return std::wstring(L"NIL");
        }
        Word type = getType(atom);
        std::wstringstream sb;
        switch(type) {
        case TAG_TYPE_NUMBER:
            sb << getNumber(atom);
            return sb.str();
        case TAG_TYPE_VALUE:
            sb << "VALUE: ";
            sb << untagIndex(atom);
            return sb.str();
        case TAG_TYPE_SYMBOL:
            sb << "SYMBOL: ";
            sb << untagIndex(atom);
            return sb.str();
        case TAG_TYPE_BIF:
            sb << "BIF: ";
            sb << untagIndex(atom);
            return sb.str();
        case TAG_TYPE_GLOBAL:
            sb << "GLOBAL:";
            sb << untagIndex(atom);
            return sb.str();
        case TAG_TYPE_CONS:
            sb << "CELL: ";
            sb << untagIndex(atom);
            return sb.str();
        default:
            return std::wstring(L"UNKNOWN");
        }
    }

    void dump(std::wostream& stream) {
        dumpCells(stream);
        dumpSymbols(stream);
        dumpBIF(stream);
        dumpGlobals(stream);
        dumpValues(stream);
    }

    void dumpCells(std::wostream& stream) {
        stream << "CELL STORAGE " << std::endl;
        stream << "------------------------------------------------------------------------------" << std::endl;
        for(unsigned int i = 0; i < cells.size(); i++) {
            Cons cons = cells[i];
            stream << std::setw(10) << i << L": [" << std::setw(30) <<  toDumpString(cons->car()) << " :: " << std::setw(30) << toDumpString(cons->cdr()) << L"]" << std::endl;
        }
    }

    void dumpSymbols(std::wostream& stream) {
        stream << "SYMBOLS " << std::endl;
        stream << "------------------------------------------------------------------------------" << std::endl;
        for(unsigned int i = 0; i < symbolTable.size(); i++) {
            stream << std::setw(10) << i << L": " << std::setw(66) <<  symbolTable.getKey(i) << std::endl;
        }
    }

    void dumpValues(std::wostream& stream) {
        stream << "STRINGS " << std::endl;
        stream << "------------------------------------------------------------------------------" << std::endl;
        for(unsigned int i = 0; i < stringTable.size(); i++) {
            if (stringTable.inUse(i)) {
                stream << std::setw(10) << i << L": " << std::setw(66) << stringTable.get(i) << std::endl;
            } else {
                stream << std::setw(78) << "FREE" << std::endl;
            }
        }
    }

    void dumpBIF(std::wostream& stream) {
        stream << "BIF " << std::endl;
        stream << "------------------------------------------------------------------------------" << std::endl;
        for(unsigned int i = 0; i < bifTable.size(); i++) {
            stream << std::setw(10) << i << L": " << std::setw(66) <<  getName(bifTable.getKey(i)) << std::endl;
        }
    }

    void dumpGlobals(std::wostream& stream) {
        stream << "GLOBALS " << std::endl;
        stream << "------------------------------------------------------------------------------" << std::endl;
        for(unsigned int i = 0; i < globalsTable.size(); i++) {
            stream << std::setw(10) << i << L": " << std::setw(30) <<  getName(globalsTable.getKey(i)) << " -> " << std::setw(32) << toDumpString(globalsTable.getValue(i)) << std::endl;
        }
    }



}
