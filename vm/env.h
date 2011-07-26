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
// Contains globally available constants and helper functions which are
// inlined for performance reasons.
// ---------------------------------------------------------------------------

#ifndef ENV_H
#define ENV_H

#include <cassert>
#include <string>
#include <sstream>

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
  Give the string class a simpler and shorter name (bad, I know).
  */
typedef std::wstring String;

/**
  Converts std::string to String
  */
inline String asString(std::string str) {
    String result;
    result.assign(str.begin(), str.end());
    return result;
}

/**
  Converts String to std::String
  */
inline std::string asStdString(String str) {
    std::string result;
    result.assign(str.begin(), str.end());
    return result;
}

/**
  Used to extract the tag from an atom.
  */
const Word TAG_MASK        = 0b1111;

/**
  Used for bit shifting when tagging and untagging atoms.
  */
const Word TAG_LENGTH      = 4;

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
  This type is only used once for the constant NIL pointer.
  */
const Word TAG_TYPE_NIL    = 0b0000;

/**
  Declares that the data part of the atom is a pointer into the symbol
  table.
  */
const Word TAG_TYPE_SYMBOL = 0b0001;

/**
  Declares that the data part of the atom is a n-bit (see below)
  signed integer
  */
const Word TAG_TYPE_NUMBER = 0b0010;

/**
  Declares that the data part of the atom is a pointer into a cell space.
  */
const Word TAG_TYPE_CONS   = 0b0011;

/**
  Declares that the data part of the atom is a pointer into the bif
  (built in functions) table.
  */
const Word TAG_TYPE_BIF    = 0b0100;

/**
  Declares that the data part of the atom is a pointer into the globals
  table.
  */
const Word TAG_TYPE_GLOBAL = 0b0101;

/**
  Declares that the data part of the atom is a pointer into the string
  table.
  */
const Word TAG_TYPE_STRING  = 0b0110;

/**
  Declares that the data part of the atom is a pointer into the large number
  table.
  */
const Word TAG_TYPE_LARGE_NUMBER  = 0b0111;

/**
  Declares that the data part of the atom is a pointer into the decimal
  number table.
  */
const Word TAG_TYPE_DECIMAL_NUMBER  = 0b1000;

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
  Op code: Branch true: If the stack top is true
  The list beneath it becomes the net code for this function.
  */
const Atom SYMBOL_OP_BT = SYMBOL(8);

/**
  Op code: Same as AP but without any arguments.
  */
const Atom SYMBOL_OP_AP0 = SYMBOL(9);

/**
  Op code: Store in env.
  */
const Atom SYMBOL_OP_ST = SYMBOL(10);

/**
  Op code: Load contents of a global
  */
const Atom SYMBOL_OP_LDG = SYMBOL(11);

/**
  Op code: Store global
  */
const Atom SYMBOL_OP_STG = SYMBOL(12);

/**
  Op code: Pushes the CAR of the stack top
  */
const Atom SYMBOL_OP_CAR = SYMBOL(13);

/**
  Op code: Pushes the CDR of the stack top
  */
const Atom SYMBOL_OP_CDR = SYMBOL(14);

/**
  Op code: Pops two values from the stack and pushes a new Cons
  */
const Atom SYMBOL_OP_CONS = SYMBOL(15);

/**
  Op code: Operator ==
  */
const Atom SYMBOL_OP_EQ = SYMBOL(16);

/**
  Op code: Operator !=
  */
const Atom SYMBOL_OP_NE = SYMBOL(17);

/**
  Op code: Operator < (only for Numbers)
  */
const Atom SYMBOL_OP_LT = SYMBOL(18);

/**
  Op code: Operator > (only for Numbers)
  */
const Atom SYMBOL_OP_GT = SYMBOL(19);

/**
  Op code: Operator <= (only for Numbers)
  */
const Atom SYMBOL_OP_LTQ = SYMBOL(20);

/**
  Op code: Operator >= (only for Numbers)
  */
const Atom SYMBOL_OP_GTQ = SYMBOL(21);

/**
  Op code: Operator + (only for Numbers)
  */
const Atom SYMBOL_OP_ADD = SYMBOL(22);

/**
  Op code: Operator - (only for Numbers)
  */
const Atom SYMBOL_OP_SUB = SYMBOL(23);

/**
  Op code: Operator * (only for Numbers)
  */
const Atom SYMBOL_OP_MUL = SYMBOL(24);

/**
  Op code: Operator / (only for Numbers)
  */
const Atom SYMBOL_OP_DIV = SYMBOL(25);

/**
  Op code: Operator % (only for Numbers)
  */
const Atom SYMBOL_OP_REM = SYMBOL(26);

/**
  Op code: Operator ! (Boolean logic on symbols TRUE and FALSE)
  */
const Atom SYMBOL_OP_NOT = SYMBOL(27);

/**
  Op code: Operator & (Boolean logic on symbols TRUE and FALSE)
  */
const Atom SYMBOL_OP_AND = SYMBOL(28);

/**
  Op code: Operator | (Boolean logic on symbols TRUE and FALSE)
  */
const Atom SYMBOL_OP_OR = SYMBOL(29);

/**
  Op code: Stops the interpreter of the current function.
  */
const Atom SYMBOL_OP_STOP = SYMBOL(30);

/**
  Op code: Pops a cons and a new value from the stack, replaces
  the car of the cons cell with the second value and pushes the
  modified cell on the stack.
  */
const Atom SYMBOL_OP_RPLACAR = SYMBOL(31);

/**
  Op code: Same as RPLACAR, but replaces the CDR of the cell.
  */
const Atom SYMBOL_OP_RPLACDR = SYMBOL(32);

/**
  Op code: Appends a value to a list. Short form of:
  #LDC list, #LDC x, #NIL, #CONS #RPLACDR
  */
const Atom SYMBOL_OP_CHAIN = SYMBOL(33);

/**
  Op code: Finishes a sequence of chains and pushes
  the resulting list onto the stack.
  */
const Atom SYMBOL_OP_CHAIN_END = SYMBOL(34);

/**
  Op code: Used to tell the VM the currently active file.
  */
const Atom SYMBOL_OP_FILE = SYMBOL(35);

/**
  Op code: Used to tell the VM the currently active line.
  */
const Atom SYMBOL_OP_LINE = SYMBOL(36);

/**
  Reads the tag of a given atom.
  */
inline Word getType(Atom atom) {
    return atom & TAG_MASK;
}

/**
  Checks whether the given atom is NIL.
  */
inline bool isNil(Atom atom) {
    return atom == NIL;
}

/**
  Checks whether the given atom is a symbol.
  */
inline bool isSymbol(Atom atom) {
    return getType(atom) == TAG_TYPE_SYMBOL;
}

/**
  Checks whether the given atom is a number.
  */
inline bool isNumber(Atom atom) {
    return getType(atom) == TAG_TYPE_NUMBER;
}

/**
  Checks whether the given atom is a cell.
  */
inline bool isCons(Atom atom) {
    return getType(atom) == TAG_TYPE_CONS;
}

/**
  Checks whether the given atom is a bif
  (built in function).
  */
inline bool isBIF(Atom atom) {
    return getType(atom) == TAG_TYPE_BIF;
}

/**
  Checks whether the given atom is a global.
  */
inline bool isGlobal(Atom atom) {
    return getType(atom) == TAG_TYPE_GLOBAL;
}

/**
  Checks whether the given atom is a string.
  */
inline bool isString(Atom atom) {
    return getType(atom) == TAG_TYPE_STRING;
}

/**
  Checks whether the given atom is a large number.
  */
inline bool isLargeNumber(Atom atom) {
    return getType(atom) == TAG_TYPE_LARGE_NUMBER;
}

/**
  Checks whether the given atom is a decimal number.
  */
inline bool isDecimalNumber(Atom atom) {
    return getType(atom) == TAG_TYPE_DECIMAL_NUMBER;
}

/**
  Checks whether the given atom is a number, large number
  or decimal number.
  */
inline bool isNumeric(Atom atom) {
    return isNumber(atom) || isLargeNumber(atom) || isDecimalNumber(atom);
}

/**
  Provides access to the unsigned index stored in the atoms data section.
  */
inline Word untagIndex(Atom atom) {
    return atom >> TAG_LENGTH;
}

/**
  Generates an atom which encodes the given index for the given table type.
  */
inline Atom tagIndex(Word index, Word type) {
    return (index << TAG_LENGTH) | type;
}

/**
  Provides information about the current state of the storage.
  */
struct StorageStatus {
    Word cellsUsed;
    Word totalCells;
    Word numSymbols;
    Word numGlobals;
    Word stringsUsed;
    Word totalStrings;
    Word numbersUsed;
    Word totalNumbers;
    Word deicmalsUsed;
    Word totalDecimals;
};

/**
  Provides information about the status of an Engine.
  */
struct EngineStatus {
    Word timeElapsed;
    Word instructionsExecuted;
    Word gcRuns;
    StorageStatus storageStats;
};

#endif // ENV_H
