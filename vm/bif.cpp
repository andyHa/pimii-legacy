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
  Compiles the given file and pushes the resulting op-codes on the stack
  (as list).
  */
Atom bif_compile(Engine* engine, Storage* storage, Atom args) {
    engine->expect(isCons(args),
                   "bif_compile called without parameters!",
                   __FILE__,
                   __LINE__);
    Atom first = storage->getCons(args)->car;
    engine->expect(isString(first),
                   "bif_compile requires a string as first parameter!",
                   __FILE__,
                   __LINE__);
    return engine->compileFile(storage->getString(first), false);
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

void Engine::initializeBIF() {
    makeBuiltInFunction(storage.makeSymbol(QString("println")), bif_println);
    makeBuiltInFunction(storage.makeSymbol(QString("asString")), bif_asString);
    makeBuiltInFunction(storage.makeSymbol(QString("parse")), bif_parse);
    makeBuiltInFunction(storage.makeSymbol(QString("compile")), bif_compile);
    makeBuiltInFunction(storage.makeSymbol(QString("strlen")), bif_strlen);
    makeBuiltInFunction(storage.makeSymbol(QString("substr")), bif_substr);
}
