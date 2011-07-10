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
// ---------------------------------------------------------------------------
// Represents the internal data storage.
// ---------------------------------------------------------------------------

#ifndef STORAGE_H
#define STORAGE_H

#include "env.h"
#include "lookuptable.h"
#include "valuetable.h"

#include <set>

#include <QReadWriteLock>

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
  Internally used to store memory cells along with a header used
  for garbage collection and free-list management.
  */
struct StorageEntry {
    volatile Word header;
    Cons cell;
};

/**
  Marks a cell as free. Used be the GC algorithm.
  */
const Word HEADER_FREE = 0b00 << (NUMBER_OF_BITS - 2);

/**
  Marks a cell as "gray" (touched). Used be the GC algorithm.
  */
const Word HEADER_GRAY = 0b01 << (NUMBER_OF_BITS - 2);

/**
  Marks a cell as "black" (in use). Used be the GC algorithm.
  */
const Word HEADER_BLACK = 0b10 << (NUMBER_OF_BITS - 2);

/**
  Marks a cell as "white" (in unused). Used be the GC algorithm.
  */
const Word HEADER_WHITE = 0b11 << (NUMBER_OF_BITS - 2);

/**
  Used to detect an index-overflow in the white-list management.
  This is required, because the highest 2 bits are used for
  coloring cells.
  */
const Word HEADER_BITS = 0b11 << (NUMBER_OF_BITS - 2);


/**
  Storage area, contains a complete storage image for
  exectuion.
  */
class Storage
{
    /**
      Declares a fixed symbol and makes sure it has the expected constant value.
      */
    void declaredFixedSymbol(Word expected, String name);

    /**
      Registers symbols which have fixed and expected index values.
      */
    void initializeSymbols();

    /**
      Maps strings to unique symbol indices
      */
    LookupTable <String, String, Word> symbolTable;

    /**
      Maps symbols to global variables.
      */
    LookupTable <Word, Atom, Word> globalsTable;

    /**
      Contains the table of used strings.
      */
    ValueTable <Word, String> stringTable;

    /**
      Points to the index of the first free cell + 1.
      If this value is 0, no cells are free, and a new one
      must be allocated.
      */
    Word freeList;

    /**
      Contains the cell storage.
      */
    std::vector <StorageEntry> cells;

    /**
      Contains the number of allocated cells.
      */
    Word allocatedCells;

    /**
      Implements the mark-phase of the garbage collector.
      */
    void mark();

    /**
      Implements the sweep-phase of the garbage collector.
      */
    void sweep();
public:
    Storage();

    /**
      Creates or looks up the symbol for the given string.
      */
    Atom makeSymbol(String name);

    /**
      Returns the name of the given symbol.
      */
    String getSymbolName(Atom symbol);

    /**
      Prepares the garbage collector...
      */
    void gcBegin();

    /**
      Used by the engine to mark a register as gc root.
      */
    void addGCRoot(Atom atom);

    /**
      Performs the garbage reclaim...
      */
    void gcComplete();

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
      Creates a new cell with car set to next. Sets the cdr of cell
      where cons points to the atom representing this cell and also
      returns this value.
      */
    Atom cons(Atom cons, Atom next);

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
    String getGlobalName(Atom atom);

    /**
      Reads the given global.
      */
    Atom readGlobal(Atom atom);

    /**
      Writes the given global.
      */
    void writeGlobal(Atom atom, Atom value);

    /**
      Generates a a value atom, pointing to the given string.
      */
    Atom makeString(String string);

    /**
      Returns the string value to which the given atom points.
      */
    String getString(Atom atom);

    /**
      Returns the total number of cells reserved.
      */
    Word getTotalCells();

    /**
      Returns the number of used (allocated) cells.
      */
    Word getAllocatedCells();

    /**
      Dumps the contents of the cell storage to the console.
      */
    void dumpCells();

};

#endif // STORAGE_H
