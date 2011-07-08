#include "storage.h"
#include "tools.h"

#include <iomanip>
#include <algorithm>

Storage::Storage()
{
    freeList = 0;
    initializeSymbols();
}

void Storage::declaredFixedSymbol(Word expected, String name) {
    Atom value = makeSymbol(name);
    assert(value == expected);
}

void Storage::initializeSymbols() {
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
    declaredFixedSymbol(SYMBOL_OP_ST, L"ST");
    declaredFixedSymbol(SYMBOL_OP_LDG, L"LDG");
    declaredFixedSymbol(SYMBOL_OP_STG, L"STG");
    declaredFixedSymbol(SYMBOL_OP_CAR, L"CAR");
    declaredFixedSymbol(SYMBOL_OP_CDR, L"CDR");
    declaredFixedSymbol(SYMBOL_OP_CONS, L"CONS");
    declaredFixedSymbol(SYMBOL_OP_EQ, L"EQ");
    declaredFixedSymbol(SYMBOL_OP_NE, L"NE");
    declaredFixedSymbol(SYMBOL_OP_LT, L"LT");
    declaredFixedSymbol(SYMBOL_OP_GT, L"GT");
    declaredFixedSymbol(SYMBOL_OP_LTQ, L"LTQ");
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
    declaredFixedSymbol(SYMBOL_OP_RPLACAR, L"RPLACAR");
    declaredFixedSymbol(SYMBOL_OP_RPLACDR, L"RPLACDR");
    declaredFixedSymbol(SYMBOL_OP_CHAIN, L"CHAIN");
    declaredFixedSymbol(SYMBOL_OP_CHAIN_END, L"CHAINEND");
    declaredFixedSymbol(SYMBOL_OP_FILE, L"FILE");
    declaredFixedSymbol(SYMBOL_OP_LINE, L"LINE");
}


Atom Storage::makeSymbol(String name) {
    Word result = symbolTable.add(name, name);
    assert(result < MAX_INDEX_SIZE);
    return tagIndex(result, TAG_TYPE_SYMBOL);
}

String Storage::getSymbolName(Atom symbol) {
    assert(isSymbol(symbol));
    return symbolTable.getKey(untagIndex(symbol));
}

Atom Storage::makeCons(Atom car, Atom cdr) {
    if (freeList > 0) {
        Word index = freeList-1;
        StorageEntry se = cells[index];
        freeList = se.header;
        cells[index].header = HEADER_GRAY;
        Cons cons = se.cell;
        cons->car = car;
        cons->cdr = cdr;
        allocatedCells++;
        return tagIndex(index, TAG_TYPE_CONS);
    } else {
        Cons cons = new Cell();
        cons->car = car;
        cons->cdr = cdr;

        StorageEntry entry;
        entry.cell = cons;
        entry.header = HEADER_GRAY;
        Word result = cells.size();
        assert((result < MAX_INDEX_SIZE) && ((result & HEADER_BITS) == 0));
        cells.push_back(entry);
        allocatedCells++;

        return tagIndex(result, TAG_TYPE_CONS);
    }
}

Cons Storage::getCons(Atom atom) {
    if (!isCons(atom)) {
        TRACE(34);
    }
    assert(isCons(atom));
    return cells[untagIndex(atom)].cell;
}

Word Storage::getAllocatedCells() {
    return allocatedCells;
}

Word Storage::getTotalCells() {
    return cells.size();
}

void Storage::gcComplete() {
    mark();
    sweep();
}

void Storage::gcBegin() {
    for(Word i = 0; i < cells.size(); i++) {
        if (cells[i].header == HEADER_GRAY || cells[i].header == HEADER_BLACK) {
            cells[i].header = HEADER_WHITE;
        }
    }
}

void Storage::addGCRoot(Atom atom) {
    if (!isCons(atom)) {
        return;
    }
    cells[untagIndex(atom)].header = HEADER_GRAY;
}

void Storage::mark() {
    Word currentIndex = getTotalCells() - 1;
    Word nextGray = currentIndex - 1;
    do {
        if (cells[currentIndex].header == HEADER_GRAY) {
            Cons cell = cells[currentIndex].cell;
            TRACE("Cell " << currentIndex << " is gray! ");
            cells[currentIndex].header = HEADER_BLACK;
            if (isCons(cell->car)) {
                Word carIdx = untagIndex(cell->car);
                if (isCons(cell->cdr)) {
                    Word cdrIdx = untagIndex(cell->cdr);
                    TRACE("Marking CAR " << carIdx << " and CDR " << cdrIdx <<  " gray... ");
                    cells[carIdx].header = HEADER_GRAY;
                    cells[cdrIdx].header = HEADER_GRAY;
                    currentIndex = std::max(nextGray, std::max(carIdx, cdrIdx));
                    nextGray = std::max(nextGray, std::min(carIdx, cdrIdx));
                    TRACE("Next index is: " << currentIndex);
                    TRACE("Next gray is: " << nextGray);
                } else {
                    TRACE("Marking CAR " << carIdx << " gray... ");
                    cells[carIdx].header = HEADER_GRAY;
                    currentIndex = std::max(nextGray, carIdx);
                    nextGray = std::min(nextGray, carIdx);
                    TRACE("Next index is: " << currentIndex);
                    TRACE("Next gray is: " << nextGray);
                }
            } else {
                if (isCons(cell->cdr)) {
                    Word cdrIdx = untagIndex(cell->cdr);
                    TRACE("Marking CDR " << cdrIdx << " gray... ");
                    cells[cdrIdx].header = HEADER_GRAY;
                    currentIndex = std::max(nextGray, cdrIdx);
                    nextGray = std::min(nextGray, cdrIdx);
                    TRACE("Next index is: " << currentIndex);
                    TRACE("Next gray is: " << nextGray);
                } else {
                    TRACE("Goto next gray: " << nextGray);
                    currentIndex = nextGray;
                    nextGray--;
                }
            }
        } else {
            TRACE("Cell " << currentIndex << " is not gray! Goto next gray: " << nextGray);
            currentIndex = nextGray;
            nextGray--;
        }
    } while(currentIndex > 0);
}

void Storage::sweep() {
    for(Word i = 0; i < cells.size(); i++) {
        if (cells[i].header == HEADER_WHITE) {
            TRACE("Added #" << i << " to free list");
            cells[i].header = freeList;
            freeList = i + 1;
            allocatedCells--;
        } else if (cells[i].header == HEADER_BLACK) {
            TRACE("Reverted #" << i << " to WHITE");
            cells[i].header = HEADER_WHITE;
        }
    }
}

String getColor(Word header) {
    if ((header & HEADER_BITS) == HEADER_FREE) {
        return String(L"FREE");
    }
    if ((header & HEADER_BITS) == HEADER_GRAY) {
        return String(L"GRAY");
    }
    if ((header & HEADER_BITS) == HEADER_BLACK) {
        return String(L"BLACK");
    }
    if ((header & HEADER_BITS) == HEADER_WHITE) {
        return String(L"WHITE");
    }
    return String(L"?");
}

void Storage::dumpCells() {
    std::wcout << "CELL STORAGE " << std::endl;
    std::wcout << "------------------------------------------------------------------------------" << std::endl;
    for(unsigned int i = 0; i < cells.size(); i++) {
        Cons cons = cells[i].cell;
        std::wcout << std::setw(10)
                   << i
                   << L": ["
                   << std::setw(20)
                   << toDumpString(cons->car)
                   << " :: "
                   << std::setw(20)
                   << toDumpString(cons->cdr)
                   << L"|"
                   << std::setw(20)
                   << getColor(cells[i].header)
                   << L"]"
                   << std::endl;
    }
}

/*

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
*/

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

String Storage::getGlobalName(Atom atom) {
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

Atom Storage::makeString(String string) {
    Word index = stringTable.allocate(string);
    assert(index < MAX_INDEX_SIZE);
    return tagIndex(index, TAG_TYPE_STRING);
}

String Storage::getString(Atom atom) {
    assert(isString(atom));
    Word index = untagIndex(atom);
    return stringTable.get(index);
}
