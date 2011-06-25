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
    Contains the memory management system used by the runtime. The memory ist
    mainly divided into 5 sections:

        * The cell storage which consits of record which each contains 2 atoms.
        * The symbol table, which maps constant strings to fixed atoms.
        * The bif table, which maps symbols to function pointers of built in
          functions
        * The globals table, which provides a mapping from symbols to atoms
          along with a storage cell for each such atom.
        * The values table which holds various binary data, like strings or
          byte arrays.

    The symbol, bif and global table never release any memory once allocated.
    The values are automatically freed, once the are no longer referenced. The
    cell storage, which makes of the central part of the system is also
    managed by a garbage collector.

    WARNING: The current implementation makes assumptions of the order of bits
    within a word and is optimized for x86/x64 architectures. On big endian
    machines, some of the functions need to be re-written. However, only this
    set of operations need to be looked at, since other parts rely on higher
    abstractions provided by this module.
*/
#ifndef UNIVERSE_H
#define UNIVERSE_H

#include <string>

namespace memory {

    /**
      Represents a machine word. According to the C++ standard, int should be
      one word in size. If not, only this type needs to be redefined but all
      uses remain valid.
      */
    typedef unsigned int Word;

    /**
      An atom is the memory unit with the finest granularity. This value is
      split up into a data section and a tag (which consumes TAG_LENGH bits).
      This tag defines the meaning of the atom.
      */
    typedef Word Atom;

    /**
      Used to extract the tag from an atom.
      */
    const Word TAG_MASK        = 0b111;

    /**
      Used for bit shifting when tagging and untagging atoms.
      */
    const Word TAG_LENGTH      = 3;

    /**
      This type is only used once for the constant NIL pointer.
      */
    const Word TAG_TYPE_NIL    = 0b000;

    /**
      Declares that the data part of the atom is a pointer into the symbol
      table.
      */
    const Word TAG_TYPE_SYMBOL = 0b001;

    /**
      Declares the the data part of the atom is a n-bit (see below)
      signed integer
      */
    const Word TAG_TYPE_NUMBER = 0b010;

    /**
      Declares the the data part of the atom is pointer into a cell space.
      */
    const Word TAG_TYPE_CONS   = 0b011;

    /**
      Declares the the data part of the atom is a pointer into the bif
      (built in functions) table.
      */
    const Word TAG_TYPE_BIF    = 0b100;

    /**
      Declares the the data part of the atom is pointer into the globals
      table.
      */
    const Word TAG_TYPE_GLOBAL = 0b101;

    /**
      Declares the the data part of the atom is pointer into the values
      table.
      */
    const Word TAG_TYPE_VALUE  = 0b110;

    /**
      Contains the number of bits in a machine word. This is used for
      bit-fiddling operations and limit checks.
      */
    const Word NUMBER_OF_BITS = sizeof(Word) * 8;

    /**
      Determines how many bits are left for the data section of an atom.
      */
    const Word EFFECTIVE_BITS = NUMBER_OF_BITS - TAG_LENGTH;

    /**
      Contains the highest table index for symbols, bifs, globals and values.
      */
    const Word MAX_INDEX_SIZE =  1 << EFFECTIVE_BITS;

    /**
      Computes the upper limit of number which can be stored in atoms.
      */
    const int MAX_SMALL_INT_SIZE = (1 << (EFFECTIVE_BITS - 1)) - 1;

    /**
      Computes the lower limit of number which can be stored in atoms.
      */
    const int MIN_SMALL_INT_SIZE = (1 << (EFFECTIVE_BITS - 1)) * -1;

    /**
      Generates a data check pattern which is used to prevent (or warn on)
      data loss.
      */
    const Word BIT_CHECK_MASK = TAG_MASK << 1 | 1;
    /**
      Enumerates the bits which will be shifted out, when tagging a number.
      These need to be all 0 or all 1, but not mixed. Otherwise data loss will
      occur.
      */
    const Word LOST_BITS = BIT_CHECK_MASK << (NUMBER_OF_BITS - TAG_LENGTH - 1);
    /**
      Sets the highest bit to 1 in order to check if a value is negative.
      */
    const Word SIGN_CHECK_BIT = 1 << (NUMBER_OF_BITS - 1);

    /**
      Cell storage is allocated blockwise. Normally the segment size shoud
      exactly match the page size of the operating system. However, since
      malloc uses some bits for bookkeeping, this constant defines by how many
      bytes a segment is reduced in order to achieve optimal performance.
      */
    const Word MALLOC_HEADER = 12;
    /**
      Determines the desired byte size of a segment of cells.
      */
    const Word SEGMENT_SIZE = 1024 * 4 - MALLOC_HEADER;
    /**
      Determines the byte size of one segment.
      */
    const Word CELL_SIZE = 2 * sizeof(Atom);
    /**
      Counts the number of cells which fit in one segment.
      */
    const Word CELLS_IN_SEGMENT = SEGMENT_SIZE / CELL_SIZE;

    /**
      Defines the constant for the NIL atom, not to be confused with the NIL
      opcode, which pushes NIL onto the stack.
      */
    const Atom NIL = 0;

    /**
      Utility macro (BOOOOO BAD, I know) which enables the computation of
      expected values for predefined symbols. This permits fast comparison of
      well known and heavily used symbols, like TRUE, FALSE and op codes.
      */
    #define SYMBOL(x) x << TAG_LENGTH | TAG_TYPE_SYMBOL

    /**
      Defines the symbol for TRUE
      */
    const Atom SYMBOL_TRUE = SYMBOL(0);

    /**
      Defines the symbol for FALSE
      */
    const Atom SYMBOL_FALSE = SYMBOL(1);

    /**
      Op code: Pushes NIL onto the stack.
      */
    const Atom SYMBOL_OP_NIL = SYMBOL(2);

    /**
      Op code: Load from the environment
      */
    const Atom SYMBOL_OP_LD = SYMBOL(3);

    /**
      Op code: Load a constant
      */
    const Atom SYMBOL_OP_LDC = SYMBOL(4);

    /**
      Op code: Load a function
      */
    const Atom SYMBOL_OP_LDF = SYMBOL(5);

    /**
      Op code: Apply a function
      */
    const Atom SYMBOL_OP_AP = SYMBOL(6);

    /**
      Op code: Return (restore calling env)
      */
    const Atom SYMBOL_OP_RTN = SYMBOL(7);

    /**
      Op code:Select in if statement
      */
    const Atom SYMBOL_OP_SEL = SYMBOL(8);

    /**
      Op code: Rejoin main control
      */
    const Atom SYMBOL_OP_JOIN = SYMBOL(9);

    /**
      Op code: Recursive apply
      */
    const Atom SYMBOL_OP_RAP = SYMBOL(10);

    /**
      Op code: Create a dummy env
      */
    const Atom SYMBOL_OP_DUM = SYMBOL(11);

    /**
      Op code: Store in env.
      */
    const Atom SYMBOL_OP_ST = SYMBOL(12);

    /**
      Op code: Load contents of a global
      */
    const Atom SYMBOL_OP_LDG = SYMBOL(13);

    /**
      Op code: Store global
      */
    const Atom SYMBOL_OP_STG = SYMBOL(14);

    /**
      Op code: Call a built in function
      */
    const Atom SYMBOL_OP_BAP = SYMBOL(15);

    /**
      Op code: Pushes the CAR of the stack top
      */
    const Atom SYMBOL_OP_CAR = SYMBOL(16);

    /**
      Op code: Pushes the CDR of the stack top
      */
    const Atom SYMBOL_OP_CDR = SYMBOL(17);

    /**
      Op code: Pops two values from the stack and pushes a new Cons
      */
    const Atom SYMBOL_OP_CONS = SYMBOL(18);

    /**
      Op code: Operator ==
      */
    const Atom SYMBOL_OP_EQ = SYMBOL(19);

    /**
      Op code: Operator !=
      */
    const Atom SYMBOL_OP_NE = SYMBOL(20);

    /**
      Op code: Operator < (only for Numbers)
      */
    const Atom SYMBOL_OP_LE = SYMBOL(21);

    /**
      Op code: Operator > (only for Numbers)
      */
    const Atom SYMBOL_OP_GT = SYMBOL(22);

    /**
      Op code: Operator <= (only for Numbers)
      */
    const Atom SYMBOL_OP_LEQ = SYMBOL(23);

    /**
      Op code: Operator >= (only for Numbers)
      */
    const Atom SYMBOL_OP_GTQ = SYMBOL(24);

    /**
      Op code: Operator + (only for Numbers)
      */
    const Atom SYMBOL_OP_ADD = SYMBOL(25);

    /**
      Op code: Operator - (only for Numbers)
      */
    const Atom SYMBOL_OP_SUB = SYMBOL(26);

    /**
      Op code: Operator * (only for Numbers)
      */
    const Atom SYMBOL_OP_MUL = SYMBOL(27);

    /**
      Op code: Operator / (only for Numbers)
      */
    const Atom SYMBOL_OP_DIV = SYMBOL(28);

    /**
      Op code: Operator % (only for Numbers)
      */
    const Atom SYMBOL_OP_REM = SYMBOL(29);

    /**
      Op code: Operator ! (Boolean logic on symbols TRUE and FALSE)
      */
    const Atom SYMBOL_OP_NOT = SYMBOL(30);

    /**
      Op code: Operator & (Boolean logic on symbols TRUE and FALSE)
      */
    const Atom SYMBOL_OP_AND = SYMBOL(31);

    /**
      Op code: Operator | (Boolean logic on symbols TRUE and FALSE)
      */
    const Atom SYMBOL_OP_OR = SYMBOL(32);

    /**
      Op code: Stops the interpreter of the current function.
      */
    const Atom SYMBOL_OP_STOP = SYMBOL(33);

    /**
      Internal data structure which represents a cell. Field access is
      encapsulated to permit reference counting for values like strings.
      */
    struct Cell {
    private:
        Atom head;
        Atom tail;
    public:
        Atom car();
        Atom cdr();
        void car(Atom atom);
        void cdr(Atom atom);
    };

    /**
      Points to a cell within the cell storage.
      */
    typedef Cell* Cons;

    /**
      Defines a built in function. Retrieves the stack stop,
      several parameters must be passed as list, and returns the
      result as list (may again be a list, or NIL to indicate that
      there is no result).
      */
    typedef Atom (*BIF)(Atom param);

    /**
      Initializes all global constants and memory pools.
      */
    void initialize();

    /**
      Registers central built in functions. There are defined in
      bif.cpp.
      */
    void initializeBIF();



    // ----------------------------------------------------------------------------
    // Statistics and Debug infos...
    // ----------------------------------------------------------------------------

    /**
      Returns the number of cells allocated.
      */
    Word getNumberOfCells();

    /**
      Returns the total amount of memory used by the cell storage.
      */
    Word getCellsMemSize();

    /**
      Returns the number of values allocated.
      */
    Word getNumberOfValues();

    /**
      Generates a short repesentation of the given atom. This is
      used when dumping various memory tables.
      */
    std::wstring toDumpString(Atom atom);

    /**
      Dumps all memory information.
      */
    void dump(std::wostream& stream);

    /**
      Dumps the contents of the cell storage.
      */
    void dumpCells(std::wostream& stream);

    /**
      Dumps the symbol table.
      */
    void dumpSymbols(std::wostream& stream);

    /**
      Dumps the globals table.
      */
    void dumpGlobals(std::wostream& stream);

    /**
      Dumps the table of built in functions.
      */
    void dumpBIF(std::wostream& stream);

    /**
      Dumps the binary values store.
      */
    void dumpValues(std::wostream& stream);
}
#endif // UNIVERSE_H
