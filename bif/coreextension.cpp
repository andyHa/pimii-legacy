#include "coreextension.h"

#include <algorithm>
#include <sstream>

CoreExtension* CoreExtension::INSTANCE = new CoreExtension();

QString CoreExtension::name() {
    return QString("CoreExtension");
}

void CoreExtension::registerBuiltInFunctions(Engine* engine) {

    // Typesystem
    engine->makeBuiltInFunction("typeOf", bif_typeOf);
    engine->makeBuiltInFunction("symbol", bif_symbol);
    engine->makeBuiltInFunction("asString", bif_asString);
    engine->makeBuiltInFunction("parse", bif_parse);

    // Compilation
    engine->makeBuiltInFunction("compile", bif_compile);
    engine->makeBuiltInFunction("include", bif_include);
    engine->makeBuiltInFunction("call", bif_call);
    engine->makeBuiltInFunction("eval", bif_eval);

    // String functions
    engine->makeBuiltInFunction("strlen", bif_strlen);
    engine->makeBuiltInFunction("substr", bif_substr);
    engine->makeBuiltInFunction("println", bif_println);
    //Split, Explode, Implode

    // Maths
    //cos sin sqrt round floor ceil pow
}

void CoreExtension::bif_println(const CallContext& ctx) {
    ctx.engine->println(
                ctx.engine->toSimpleString(
                    ctx.fetchArgument(BIF_INFO)));
}

void CoreExtension::bif_typeOf(const CallContext& ctx) {
    Atom first = ctx.fetchArgument(BIF_INFO);
    switch(getType(first)) {
    case TAG_TYPE_CONS:
        ctx.setResult(SYMBOL_TYPE_CONS);
    case TAG_TYPE_DECIMAL_NUMBER:
        ctx.setResult(SYMBOL_TYPE_DECIMAL);
    case TAG_TYPE_LARGE_NUMBER:
    case TAG_TYPE_NUMBER:
        ctx.setResult(SYMBOL_TYPE_NUMBER);
    case TAG_TYPE_REFERENCE:
        ctx.setResult(SYMBOL_TYPE_REFERENCE);
    case TAG_TYPE_STRING:
        ctx.setResult(SYMBOL_TYPE_STRING);
    case TAG_TYPE_SYMBOL:
        ctx.setResult(SYMBOL_TYPE_SYMBOL);
    case TAG_TYPE_BIF:
        ctx.setResult(SYMBOL_TYPE_BIF);
    case TAG_TYPE_GLOBAL:
        ctx.setResult(SYMBOL_TYPE_GLOBAL);
    }
}

void CoreExtension::bif_symbol(const CallContext& ctx) {
    ctx.setResult(ctx.storage->makeSymbol(ctx.fetchString(BIF_INFO)));
}

void CoreExtension::bif_asString(const CallContext& ctx) {
    ctx.setStringResult(
                ctx.engine->toSimpleString(ctx.fetchArgument(BIF_INFO)));
}

void CoreExtension::bif_parse(const CallContext& ctx) {
    QString str = ctx.fetchString(BIF_INFO);
    bool ok = false;

    int value = str.toInt(&ok);
    if (ok) {
        ctx.setNumberResult(value);
        return;
    }

    double doubleValue = str.toDouble(&ok);
    if (ok) {
        ctx.setDoubleResult(doubleValue);
        return;
    }

}

void CoreExtension::bif_include(const CallContext& ctx) {
    Atom code = ctx.engine->compileFile(ctx.fetchString(BIF_INFO),
                                    false);
    if (!isNil(code)) {
        ctx.engine->call(code);
    }
}

void CoreExtension::bif_compile(const CallContext& ctx) {
    QString code = ctx.fetchString(BIF_INFO);
    bool silent = false;
    if (ctx.hasMoreArguments()) {
        silent = (ctx.fetchArgument(BIF_INFO) == SYMBOL_TRUE);
    }
    ctx.setResult(ctx.engine->compileSource(QString("(eval)"),
                                 code,
                                 false,
                                 silent));
}

void CoreExtension::bif_eval(const CallContext& ctx) {
    QString code = ctx.fetchString(BIF_INFO);
    bool silent = false;
    if (ctx.hasMoreArguments()) {
        silent = (ctx.fetchArgument(BIF_INFO) == SYMBOL_TRUE);
    }
    Atom compiled = ctx.engine->compileSource(QString("(eval)"),
                                      code,
                                      false,
                                      silent);
    if (!isNil(compiled)) {
        ctx.engine->call(compiled);
    }
}

void CoreExtension::bif_call(const CallContext& ctx) {
    ctx.engine->call(ctx.fetchList(BIF_INFO));
}

void CoreExtension::bif_strlen(const CallContext& ctx) {
    ctx.setNumberResult(ctx.fetchString(BIF_INFO).length());
}

void CoreExtension::bif_substr(const CallContext& ctx) {
    QString str = ctx.fetchString(BIF_INFO);
    int pos = ctx.fetchNumber(BIF_INFO);
    pos = std::max(std::min(pos - 1, str.length()), 0);
    int length = ctx.fetchNumber(BIF_INFO);
    length = std::min(length, str.length() - pos);
    ctx.setStringResult(str.mid(pos, length));
}
