#include "coreextension.h"

#include <algorithm>
#include <sstream>

#include <QDateTime>

CoreExtension* CoreExtension::INSTANCE = new CoreExtension();

QString CoreExtension::name() {
    return QString("CoreExtension");
}

Logger CoreExtension::log("PIMII");

void CoreExtension::registerBuiltInFunctions(Engine* engine) {

    // Typesystem
    engine->makeBuiltInFunction("typeOf", bif_typeOf);
    engine->makeBuiltInFunction("asSymbol", bif_symbol);
    engine->makeBuiltInFunction("asString", bif_asString);
    engine->makeBuiltInFunction("parse", bif_parse);

    // Compilation
    engine->makeBuiltInFunction("compile", bif_compile);
    engine->makeBuiltInFunction("include", bif_include);
    engine->makeBuiltInFunction("call", bif_call);
    engine->makeBuiltInFunction("eval", bif_eval);

    // Array functions
    engine->makeBuiltInFunction("makeArray", bif_makeArray);
    engine->makeBuiltInFunction("readArray", bif_readArray);
    engine->makeBuiltInFunction("writeArray", bif_writeArray);

    // String functions
    engine->makeBuiltInFunction("strLength", bif_strlen);
    engine->makeBuiltInFunction("strPart", bif_substr);
    engine->makeBuiltInFunction("ascii", bif_ascii);
    engine->makeBuiltInFunction("char", bif_char);

    // Maths
    //cos sin sqrt round floor ceil pow

    // Specials
    engine->makeBuiltInFunction("log", bif_log);
    engine->makeBuiltInFunction("time", bif_time);
    engine->makeBuiltInFunction("version", bif_version);
    engine->makeBuiltInFunction("wordsize", bif_wordsize);
    engine->makeBuiltInFunction("engine::setValue", bif_setValue);
    engine->makeBuiltInFunction("engine::getValue", bif_getValue);
    engine->makeBuiltInFunction("engine::getValueKeys", bif_getValueKeys);
    engine->makeBuiltInFunction("settings::read", bif_readSetting);
    engine->makeBuiltInFunction("settings::write", bif_writeSetting);
}

void CoreExtension::bif_setValue(const CallContext& ctx) {
    Atom name = ctx.fetchArgument(BIF_INFO);
    Atom value = ctx.fetchArgument(BIF_INFO);
    ctx.engine->setValue(name, value);
}

void CoreExtension::bif_getValue(const CallContext& ctx) {
    ctx.setResult(ctx.engine->getValue(ctx.fetchArgument(BIF_INFO)));
}

void CoreExtension::bif_getValueKeys(const CallContext& ctx) {
    ListBuilder lb(ctx.storage);
    lb.append(SYMBOL_VALUE_HOME_PATH);
    lb.append(SYMBOL_VALUE_OP_COUNT);
    lb.append(SYMBOL_VALUE_GC_COUNT);
    lb.append(SYMBOL_VALUE_GC_EFFICIENCY);
    lb.append(SYMBOL_VALUE_NUM_GC_ROOTS);
    lb.append(SYMBOL_VALUE_NUM_SYMBOLS);
    lb.append(SYMBOL_VALUE_NUM_GLOBALS);
    lb.append(SYMBOL_VALUE_NUM_TOTAL_CELLS);
    lb.append(SYMBOL_VALUE_NUM_CELLS_USED);
    lb.append(SYMBOL_VALUE_NUM_TOTAL_STRINGS);
    lb.append(SYMBOL_VALUE_NUM_STRINGS_USED);
    lb.append(SYMBOL_VALUE_NUM_TOTAL_NUMBERS);
    lb.append(SYMBOL_VALUE_NUM_NUMBERS_USED);
    lb.append(SYMBOL_VALUE_NUM_TOTAL_DECIMALS);
    lb.append(SYMBOL_VALUE_NUM_DECIMALS_USED);
    lb.append(SYMBOL_VALUE_NUM_TOTAL_REFERENCES);
    lb.append(SYMBOL_VALUE_NUM_REFERENCES_USED);
    lb.append(SYMBOL_VALUE_NUM_TOTAL_ARRAYS);
    lb.append(SYMBOL_VALUE_NUM_ARRAYS_USED);
    ctx.setResult(lb.getResult());
}

void CoreExtension::bif_time(const CallContext& ctx) {
    ctx.setNumberResult(QDateTime::currentMSecsSinceEpoch());
}

void CoreExtension::bif_version(const CallContext& ctx) {
    ctx.setStringResult(QString("BUILT - ") +
                        QString(__DATE__) +
                        " " +
                        __TIME__);
}

void CoreExtension::bif_wordsize(const CallContext& ctx) {
    ctx.setNumberResult(NUMBER_OF_BITS);
}

void CoreExtension::bif_log(const CallContext& ctx) {
    INFO(log, ctx.engine->toSimpleString(
                    ctx.fetchArgument(BIF_INFO)));
}

void CoreExtension::bif_typeOf(const CallContext& ctx) {
    Atom first = ctx.fetchArgument(BIF_INFO);
    switch(getType(first)) {
    case TAG_TYPE_CONS:
        ctx.setResult(SYMBOL_TYPE_CONS);
        return;
    case TAG_TYPE_DECIMAL_NUMBER:
        ctx.setResult(SYMBOL_TYPE_DECIMAL);
        return;
    case TAG_TYPE_LARGE_NUMBER:
    case TAG_TYPE_NUMBER:
        ctx.setResult(SYMBOL_TYPE_NUMBER);
        return;
    case TAG_TYPE_REFERENCE:
        ctx.setResult(SYMBOL_TYPE_REFERENCE);
        return;
    case TAG_TYPE_STRING:
        ctx.setResult(SYMBOL_TYPE_STRING);
        return;
    case TAG_TYPE_SYMBOL:
        ctx.setResult(SYMBOL_TYPE_SYMBOL);
        return;
    case TAG_TYPE_BIF:
        ctx.setResult(SYMBOL_TYPE_BIF);
        return;
    case TAG_TYPE_GLOBAL:
        ctx.setResult(SYMBOL_TYPE_GLOBAL);
        return;
    }
}

void CoreExtension::bif_symbol(const CallContext& ctx) {
    ctx.setResult(ctx.storage->makeSymbol(ctx.fetchString(BIF_INFO)));
}

void CoreExtension::bif_ascii(const CallContext& ctx) {
    ctx.setNumberResult(ctx.fetchString(BIF_INFO).at(0).toAscii());
}

void CoreExtension::bif_char(const CallContext& ctx) {
    ctx.setStringResult(QString(QChar(ctx.fetchNumber(BIF_INFO))));
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


void CoreExtension::bif_makeArray(const CallContext& ctx) {
    int size = 10;
    if (ctx.hasMoreArguments()) {
        size = ctx.fetchNumber(BIF_INFO);
    }
    ctx.setResult(ctx.storage->makeArray(size));
}

void CoreExtension::bif_readArray(const CallContext& ctx) {
    Array* array = ctx.fetchArray(BIF_INFO);
    int pos = ctx.fetchNumber(BIF_INFO);
    ctx.setResult(array->at(pos));
}

void CoreExtension::bif_writeArray(const CallContext& ctx) {
    Array* array = ctx.fetchArray(BIF_INFO);
    int pos = ctx.fetchNumber(BIF_INFO);
    Atom val = ctx.fetchArgument(BIF_INFO);
    array->put(pos, val);
    ctx.setResult(val);
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

QVariant CoreExtension::fetchQVariant(const CallContext& ctx,
                                      const char* bifName,
                                      const char* file,
                                      int line) {
    if (!ctx.hasMoreArguments()) {
        return QVariant();
    }
    Atom value = ctx.fetchArgument(bifName, file, line);
    if (isNil(value)) {
        return QVariant();
    } else if (isNumber(value)) {
        return QVariant((int)ctx.storage->getNumber(value));
    } else if (isSymbol(value)) {
        return QVariant(ctx.engine->toString(value));
    } else {
       return QVariant(ctx.engine->toSimpleString(value));
    }
}

void CoreExtension::bif_readSetting(const CallContext& ctx) {
    QString name = ctx.fetchString(BIF_INFO);
    QVariant defaultValue = fetchQVariant(ctx, BIF_INFO);
    QVariant val = ctx.engine->getSettings()->value(name, defaultValue);
    if (val.isNull()) {
        ctx.setResult(NIL);
    } else if (val.type() == QVariant::Int) {
        ctx.setNumberResult(val.toInt());
    } else if (val.type() == QVariant::String) {
        QString value = val.toString();
        if (value.startsWith("#")) {
            ctx.setResult(ctx.storage->makeSymbol(
                              value.right(value.length() - 1)));
        } else {
            ctx.setStringResult(value);
        }
    }
}

void CoreExtension::bif_writeSetting(const CallContext& ctx) {
    QString name = ctx.fetchString(BIF_INFO);
    QVariant value = fetchQVariant(ctx, BIF_INFO);
    ctx.engine->getSettings()->setValue(name, value);
    ctx.engine->getSettings()->sync();
    ctx.setResult(NIL);
}

