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

#include "env.h"
#include "engine.h"

#include <algorithm>
#include <sstream>

/**
  Prints the given argument to the console and appends a
  linebreak.
 */
Atom bif_println(Engine* engine, Storage* storage, Atom args) {
    if (isCons(args)) {
        engine->println(engine->toSimpleString(storage->getCons(args)->car));
    } else if (!isNil(args)) {
        engine->println(engine->toSimpleString(args));
    }
    return NIL;
}

/**
  Determines the type of the given argument and pushes an appropriate
  symbol on the stack.
 */
Atom bif_typeOf(Engine* engine, Storage* storage, Atom args) {
    if (isCons(args)) {
        switch(getType(storage->getCons(args)->car)) {
        case TAG_TYPE_CONS:
            return SYMBOL_TYPE_CONS;
        case TAG_TYPE_DECIMAL_NUMBER:
            return SYMBOL_TYPE_DECIMAL;
        case TAG_TYPE_LARGE_NUMBER:
        case TAG_TYPE_NUMBER:
            return SYMBOL_TYPE_NUMBER;
        case TAG_TYPE_REFERENCE:
            return SYMBOL_TYPE_REFERENCE;
        case TAG_TYPE_STRING:
            return SYMBOL_TYPE_STRING;
        case TAG_TYPE_SYMBOL:
            return SYMBOL_TYPE_SYMBOL;
        case TAG_TYPE_BIF:
            return SYMBOL_TYPE_BIF;
        case TAG_TYPE_GLOBAL:
            return SYMBOL_TYPE_GLOBAL;
        default:
            return NIL;
        }
    }
    return NIL;
}

/**
  Converts the given argument into a string which will be
  pushed on the stack.
 */
Atom bif_asString(Engine* engine, Storage* storage, Atom args) {
    if (!isCons(args)) {
        return storage->makeString(engine->toSimpleString(args));
    } else {
        Atom first = storage->getCons(args)->car;
        return storage->makeString(engine->toSimpleString(first));
    }
}

/**
  Parses the given string into a number and pushes the
  result on the stack.
 */
Atom bif_parse(Engine* engine, Storage* storage, Atom args) {
    engine->expect(isCons(args),
                   "bif_parse called without parameters!",
                   __FILE__,
                   __LINE__);
    Atom first = storage->getCons(args)->car;
    engine->expect(isString(first),
                   "bif_parse requires a string as first parameter!",
                   __FILE__,
                   __LINE__);
    QString str = storage->getString(first);
    int value = str.toInt();
    return storage->makeNumber(value);
}

/**
  Compiles and executes the given file. If this file was already included
  nothing will happen.
  */
Atom bif_include(Engine* engine, Storage* storage, Atom args) {
    engine->expect(isCons(args),
                   "bif_include called without parameters!",
                   __FILE__,
                   __LINE__);
    Atom first = storage->getCons(args)->car;
    engine->expect(isString(first),
                   "bif_include requires a string as first parameter!",
                   __FILE__,
                   __LINE__);
    Atom code = engine->compileFile(storage->getString(first), false);
    if (!isNil(code)) {
        engine->call(code);
    }
    return NIL;
}

/**
  Compiles the given string and pushes the resulting op-codes on the stack
  (as list).
  */
Atom bif_compile(Engine* engine, Storage* storage, Atom args) {
    engine->expect(isCons(args),
                   "bif_compile called without parameters!",
                   __FILE__,
                   __LINE__);
    Cons param = storage->getCons(args);
    Atom first = param->car;
    engine->expect(isString(first),
                   "bif_compile requires a string as first parameter!",
                   __FILE__,
                   __LINE__);
    bool silent = false;
    if (isCons(param->cdr)) {
        param = storage->getCons(param->cdr);
        silent = (param->car == SYMBOL_TRUE);
    }
    return engine->compileSource(QString("(eval)"),
                                 storage->getString(first),
                                 false,
                                 silent);
}

/**
  Compiles and evaluates the given string
  */
Atom bif_eval(Engine* engine, Storage* storage, Atom args) {
    engine->expect(isCons(args),
                   "bif_eval called without parameters!",
                   __FILE__,
                   __LINE__);
    Cons param = storage->getCons(args);
    Atom first = param->car;
    engine->expect(isString(first),
                   "bif_eval requires a string as first parameter!",
                   __FILE__,
                   __LINE__);
    bool silent = false;
    if (isCons(param->cdr)) {
        param = storage->getCons(param->cdr);
        silent = (param->car == SYMBOL_TRUE);
    }
    Atom code = engine->compileSource(QString("(eval)"),
                                      storage->getString(first),
                                      false,
                                      silent);
    if (!isNil(code)) {
        engine->call(code);
    }
    return NIL;
}

/**
  Calls the given list as function with no arguments.
  */
Atom bif_call(Engine* engine, Storage* storage, Atom args) {
    engine->expect(isCons(args),
                   "bif_call called without parameters!",
                   __FILE__,
                   __LINE__);
    Cons param = storage->getCons(args);
    Atom first = param->car;
    engine->expect(isCons(first),
                   "bif_call requires a list as first parameter!",
                   __FILE__,
                   __LINE__);
    engine->call(first);
    return NIL;
}

/**
  Returns the length of a given string.
  */
Atom bif_strlen(Engine* engine, Storage* storage, Atom args) {
    engine->expect(isCons(args),
                   "bif_strlen called without parameters!",
                   __FILE__,
                   __LINE__);
    Atom first = storage->getCons(args)->car;
    engine->expect(isString(first),
                   "bif_strlen requires a string as first parameter!",
                   __FILE__,
                   __LINE__);
    return storage->makeNumber(storage->getString(first).length());
}
/**
  Returns a substring of the given string.
  */
Atom bif_substr(Engine* engine, Storage* storage, Atom args) {
    engine->expect(isCons(args),
                   "bif_substr called without parameters!",
                   __FILE__,
                   __LINE__);
    Cons param = storage->getCons(args);
    Atom first = param->car;
    engine->expect(isString(first),
                   "bif_substr requires a string as first parameter!",
                   __FILE__,
                   __LINE__);
    Atom next = param->cdr;
    engine->expect(isCons(next),
                   "bif_substr needs 3 parameters!",
                   __FILE__,
                   __LINE__);
    param = storage->getCons(next);
    Atom second = param->car;
    engine->expect(isNumber(second),
                   "bif_strlen requires a number as second parameter!",
                   __FILE__,
                   __LINE__);
    next = param->cdr;
    engine->expect(isCons(next),
                   "bif_substr needs 3 parameters!",
                   __FILE__,
                   __LINE__);
    param = storage->getCons(next);
    Atom third = param->car;
    engine->expect(isNumber(third),
                   "bif_strlen requires a number as third parameter!",
                   __FILE__,
                   __LINE__);
    QString str = storage->getString(first);
    Word pos = std::min((int)storage->getNumber(second) - 1, str.length());
    Word length = std::min((Word)storage->getNumber(third), str.length() - pos);
    return storage->makeString(str.mid(pos, length));
}

Atom bif_dummy(Engine* engine, Storage* storage, Atom args) {
    return storage->makeReference(new DummyReference(QString("Hallo")));
}

void Engine::initializeBIF() {
    makeBuiltInFunction(storage.makeSymbol(QString("typeOf")), bif_typeOf);
    makeBuiltInFunction(storage.makeSymbol(QString("println")), bif_println);
    makeBuiltInFunction(storage.makeSymbol(QString("asString")), bif_asString);
    makeBuiltInFunction(storage.makeSymbol(QString("parse")), bif_parse);
    makeBuiltInFunction(storage.makeSymbol(QString("include")), bif_include);
    makeBuiltInFunction(storage.makeSymbol(QString("compile")), bif_compile);
    makeBuiltInFunction(storage.makeSymbol(QString("call")), bif_call);
    makeBuiltInFunction(storage.makeSymbol(QString("eval")), bif_eval);
    makeBuiltInFunction(storage.makeSymbol(QString("strlen")), bif_strlen);
    makeBuiltInFunction(storage.makeSymbol(QString("substr")), bif_substr);
    makeBuiltInFunction(storage.makeSymbol(QString("dumm")), bif_dummy);
}
