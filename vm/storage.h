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
  ---------------------------------------------------------------------------
  Represents the internal data storage.
  ---------------------------------------------------------------------------
  */

#ifndef STORAGE_H
#define STORAGE_H

#include "vm/env.h"
#include "vm/lookuptable.h"
#include "vm/valuetable.h"
#include "vm/reference.h"
#include "tools/logger.h"
#include "tools/average.h"

#include <QSharedPointer>

#include <set>
#include <deque>

/**
  Represents the central unit of memory management. All data
  sturctures are made of of cells which contain two atoms.
  since one or both of these can point to other cells,
  almost any kind of data structure can be generated.
  */
struct Cell {
    Atom car;
    Atom cdr;
};

/**
  Represents the state of an entry (Used by the garbage collector).
  */
enum EntryState {
    /**
      The entry is free.
      */
    UNUSED,
    /**
      The entry was allocated but might become free within the next GC.
      */
    GRAY,
    /**
      The entry is referenced, but its contents are not checked.
      */
    REFERENCED,
    /**
      The entry is used and its contents are checked.
      */
    CHECKED
};

/**
  Forward reference. See below.
  */
class AtomRef;

/**
  Storage area, contains a complete storage image for
  exectuion.
  */
class Storage
{
    /**
      Declares a fixed symbol and makes sure it has the expected constant value.
      */
    void declaredFixedSymbol(Word expected, const char* name);

    /**
      Registers symbols which have fixed and expected index values.
      */
    void initializeSymbols();

    /**
      Contains the logger used by the storage engine.
      */
    Logger log;

    /**
      Maps Strings to unique symbol indices
      */
    LookupTable <QString, QString, Word> symbolTable;

    /**
      Maps symbols to global variables.
      */
    LookupTable <Word, Atom, Word> globalsTable;

    /**
      Contains the table of used strings.
      */
    ValueTable <Word, QString> stringTable;

    /**
      Contains the table of large numbers.
      */
    ValueTable <Word, long> largeNumberTable;

    /**
      Contains the table of decimal numbers.
      */
    ValueTable <Word, double> decimalNumberTable;

    /**
      Contains the table of references
      */
    ValueTable < Word, QSharedPointer<Reference> > referenceTable;

    /**
      Contains the cell storage.
      */
    Cell* cells;

    /**
      Contains the state for each cell.
      */
    EntryState* states;

    /**
      Contains the size of the cells array. (Number of elements, not byte size).
      */
    Word cellSize;

    /**
      Contains the number of cells currently in use.
      */
    Word cellsInUse;

    /**
      Counts the number of executed garbage collections.
      */
    Word gcCounter;

    /**
      Contains the average ratio of freed cells within a GC run.
      */
    DoubleAverage avgGCEfficiency;

    /**
      Points to the first free cell or == cellSize, when no more cells are free.
      If it points to a cell, this cells car value points to the next free cell.
      */
    Word nextFree;

    /**
      Contains all external references to atoms.
      */
    std::set<AtomRef*> strongReferences;

    /**
      Increments the location in the given value table if the given
      atom points to one.
      */
    void incValueTable(Atom atom, Word idx);

    /**
      Invokes the garbage collector.
      */
    void gc(bool major, Atom car, Atom cdr);

    /**
      Used by the mark phase to mark one cell as checked, and mark referenced
      cells as referenced.
      */
    void markCell(Word index, std::deque<Word>& refQueue, bool alwaysQueue);

    /**
      Implements the mark-phase of the garbage collector.
      */
    void mark();

    /**
      Implements the sweep-phase of the garbage collector.
      */
    void sweep();


    friend class AtomRef;

    Q_DISABLE_COPY(Storage)
public:
    Storage();

    /**
      Creates or looks up the symbol for the given string.
      */
    Atom makeSymbol(const QString& name);

    /**
      Returns the name of the given symbol.
      */
    QString getSymbolName(Atom symbol);

    /**
      Generates a new cell, initialized with the two given atoms.
      */
    Atom makeCons(Atom car, Atom cdr);

    /**
      Replaces the CAR value of the given atom.
      */
    void setCAR(Atom atom, Atom car);

    /**
      Replaces the CDR value of the given atom.
      */
    void setCDR(Atom atom, Atom cdr);

    /**
      Creates a new cell with car set to next. Sets the cdr of tail
      to the atom representing created cell and also returns this value.
      */
    Atom append(Atom tail, Atom next);

    /**
      Returns the cell on which atom points.
      */
    inline Cell getCons(Atom atom) {
        assert(isCons(atom));
        return cells[untagIndex(atom)];
    }

    /**
      Returns and atom pointing to the global with the given name.
      */
    Atom findGlobal(Atom nameSymbol);

    /**
      Returns the name of the given global
      */
    QString getGlobalName(Atom atom);

    /**
      Reads the given global.
      */
    Atom readGlobal(Atom atom);

    /**
      Writes the given global.
      */
    void writeGlobal(Atom atom, Atom value);

    /**
      Returns the string value to which the given atom points.
      */
    QString getString(Atom atom);

    /**
      Generates a value atom, pointing to the given string.
      */
    Atom makeString(const QString& string);

    /**
      Returns the number value to which the given atom points.
      */
    long getNumber(Atom atom);

    /**
      Generates a value atom, pointing to the given string.
      */
    Atom makeNumber(long value);

    /**
      Returns the double value to which the given atom points.
      */
    double getDecimal(Atom atom);

    /**
      Generates a value atom, pointing to the given double.
      */
    Atom makeDecimal(double value);

    /**
      Returns the reference to which the given atom points.
      */
    Reference* getReference(Atom atom);

    /**
      Generates a value atom, pointing to the given reference.
      */
    Atom makeReference(Reference* value);

    /**
      Creates a new GC-root reference. This is initialized with the given
      atom. If no atom is available, NIL can be used.
      */
    AtomRef* ref(Atom atom);

    /**
      Returns the number of executed GCs.
      */
    Word statusNumGC() {
        return gcCounter;
    }

    double statusGCEfficienty() {
        return avgGCEfficiency.average();
    }

    /**
      Returns the count of GC roots. (AtomRefs)
      */
    Word statusNumGCRoots() {
        return strongReferences.size();
    }

    /**
      Returns the size of the symbol table.
      */
    Word statusNumSymbols() {
        return symbolTable.size();
    }

    /**
      Returns the number of global variables.
      */
    Word statusNumGlobals() {
        return globalsTable.size();
    }

    /**
      Returns the size of the cell storage.
      */
    Word statusTotalCells() {
        return cellSize;
    }

    /**
      Returns the number of reachable cells.
      */
    Word statusCellsUsed() {
        return cellsInUse;
    }

    /**
      Returns the total size of the strings table.
      */
    Word statusTotalStrings() {
        return stringTable.getTotalCells();
    }

    /**
      Returns the number of utilized strings.
      */
    Word statusStringsUsed() {
        return stringTable.getNumberOfUsedCells();
    }

    /**
      Returns the total size of the large number table.
      */
    Word statusTotalNumbers() {
        return largeNumberTable.getTotalCells();
    }

    /**
      Returns the number of utilized large numbers.
      */
    Word statusNumbersUsed() {
        return largeNumberTable.getNumberOfUsedCells();
    }

    /**
      Returns the total size of the decimal number table.
      */
    Word statusDecimalsUsed() {
        return decimalNumberTable.getTotalCells();
    }

    /**
      Returns the number of utilized decimal numbers.
      */
    Word statusTotalDecimals() {
        return decimalNumberTable.getNumberOfUsedCells();
    }

    /**
      Returns the size of the reference table.
      */
    Word statusReferencesUsed() {
        return referenceTable.getTotalCells();
    }

    /**
      Returns the number of utilized references.
      */
    Word statusTotalReferences() {
        return referenceTable.getNumberOfUsedCells();
    }

};


/**
  Describes a "strong" reference to an atom. Strong means, that it will
  prevent the atom from beeing garbage collected while referenced.
  */
class AtomRef {
private:
    Storage* storage;
    Atom referencedAtom;

    Q_DISABLE_COPY(AtomRef)
public:
    AtomRef(Storage* storage, Atom atom) :
        storage(storage),
        referencedAtom(atom) {
        storage->strongReferences.insert(this);
    }

    inline Atom atom() {
        return referencedAtom;
    }

    inline void atom(Atom atom) {
        referencedAtom = atom;
    }

    ~AtomRef() {
        storage->strongReferences.erase(this);
    }

};


#endif // STORAGE_H
