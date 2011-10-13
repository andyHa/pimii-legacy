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

#include "env.h"
#include "lookuptable.h"
#include "valuetable.h"
#include "reference.h"

#include <QSharedPointer>

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
  Points to a cell within the cell storage.
  */
typedef Cell* Cons;

/**
  Represents the state of an entry (Used by the garbage collector).
  */
enum EntryState {
    /**
      The entry is currently unused.
      */
    UNUSED,
    /**
      The entry is referenced, but its contents are not checked.
      */
    REFERENCED,
    /**
      The entr is used and its contents are checked.
      */
    CHECKED
};

/**
  Internally used to store memory cells along with a header used
  for garbage collection and free-list management.
  */
struct StorageEntry {
    EntryState state;
    Cons cell;
};

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
    std::vector<StorageEntry> cells;

    /**
      Contains indices of unused cells.
      */
    std::vector<Word> freeList;

    /**
      Increments the location in the given value table if the given
      atom points to one.
      */
    void incValueTable(Atom atom, Word idx);

    /**
      Implements the mark-phase of the garbage collector.
      */
    void mark();

    /**
      Implements the sweep-phase of the garbage collector.
      */
    void sweep();

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
      Runs the GC with the given root nodes.
      */
    void gc(Atom root1,Atom root2, Atom root3, Atom root4, Atom root5);

    /**
      Generates a new cell, initialized with car and NIL
      */
    std::pair<Atom, Cons> cons(Atom car, Atom cdr);

    /**
      Generates a new cell, initialized with the two given atoms.
      */
    Atom makeCons(Atom car, Atom cdr);

    /**
      Creates a new cell with car set to next. Sets the cdr of cons
      to the atom representing this cell and also returns this value.
      */
    Atom cons(Cons cons, Atom next);

    /**
      Returns the cell on which atom points.
      */
    Cons getCons(Atom atom);

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
      Returns the reference to which the given atom points. The pointer
      contained in the result must to be extracted and added to another
      QSharedPointer! If you want to use the same value, always use
      makeReference with the result. Otherwise memory corruption will occur
      and the VM will crash!
      */
    QSharedPointer<Reference> getReference(Atom atom);

    /**
      Generates a value atom, pointing to the given reference.
      */
    Atom makeReference(const QSharedPointer<Reference>& value);

    /**
      Returns the current status of the storage.
      */
    StorageStatus getStatus();

};

#endif // STORAGE_H
