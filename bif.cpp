#include "env.h"
#include "engine.h"

/**
  Prints the given argument to the console.
 */
Atom print(Engine* engine, Storage& storage, Atom args) {
    std::wcout << engine->toSimpleString(args);
    return NIL;
}

/**
  Prints the given argument to the console and appends a
  linebreak.
 */
Atom println(Engine* engine, Storage& storage, Atom args) {
    if (isNil(args)) {
        std::wcout << std::endl;
    } else {
        std::wcout << engine->toSimpleString(args) << std::endl;
    }
    return NIL;
}

/**
  Converts the given argument into a string which will be
  pushed on the stack.
 */
Atom asString(Engine* engine, Storage& storage, Atom args) {
    return storage.makeString(engine->toSimpleString(args));
}

/**
  Parses the given string into a number and pushes the
  result on the stack.
 */
Atom parse(Engine* engine, Storage& storage, Atom args) {
    String str = storage.getString(args);
    std::wstringstream buffer;
    buffer << str;
    int value;
    buffer >> value;
    return makeNumber(value);
}

void Engine::initializeBIF() {
    makeBuiltInFunction(storage.makeSymbol(String(L"print")), print);
    makeBuiltInFunction(storage.makeSymbol(String(L"println")), println);
    makeBuiltInFunction(storage.makeSymbol(String(L"asString")), asString);
    makeBuiltInFunction(storage.makeSymbol(String(L"parse")), parse);
}
