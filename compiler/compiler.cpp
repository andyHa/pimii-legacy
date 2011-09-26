#include "compiler.h"

Compiler::Compiler(const QString& fileName,
                   const QString& input,
                   Engine* engine) :
    engine(engine),
    tokenizer(new Tokenizer(input, true))
{
    file = engine->storage.makeSymbol(fileName);
}


void Compiler::addError(const InputToken& token, const char* errorMsg) {
    addError(token, QString(errorMsg));
}

void Compiler::addError(const InputToken& token, const QString& errorMsg) {
    CompilationError e;
    e.error = errorMsg;
    e.line = token.line;
    e.pos = token.pos;
    e.severe = true;
    errors.push_back(e);
}


void Compiler::expect(InputTokenType tt, const char* rep) {
    if (tokenizer->getCurrent().type != tt) {
        addError(tokenizer->getCurrent(),
                 QString("Unexpected token: ") +
                 tokenizer->getCurrentString() +
                 QString(". Expected: ")
                 +QString(rep));
    } else {
        tokenizer->fetch();
    }
}

void Compiler::addCode(Atom atom) {
    if (isNil(code)) {
        code = engine->storage.makeCons(atom, NIL);
        tail = code;
    } else {
        tail = engine->storage.cons(engine->storage.getCons(tail), atom);
    }
}

std::pair<Atom, std::vector<CompilationError> > Compiler::compile(bool appendStop) {
    code = NIL;
    tokenizer->fetch();
    addCode(SYMBOL_OP_FILE);
    addCode(file);
    while(tokenizer->getCurrent().type != TT_EOF) {
        block();
        if (tokenizer->getCurrent().type != TT_EOF) {
            addError(tokenizer->getCurrent(), "Missing Semicolon!");
        }
    }
    if (appendStop) {
        addCode(SYMBOL_OP_STOP);
    } else {
        addCode(SYMBOL_OP_RTN);
    }
    return std::pair<Atom, std::vector<CompilationError> >(code, errors);
}

void Compiler::block() {
    statement();
    while(tokenizer->isCurrent(TT_SEMICOLON)
          && !tokenizer->isLookahead(TT_R_BRACE)
          && !tokenizer->isLookahead(TT_R_BRACKET)
          && !tokenizer->isLookahead(TT_EOF)) {
        tokenizer->fetch();
        statement();
    }
    if (tokenizer->isCurrent(TT_SEMICOLON)) {
        tokenizer->fetch();
    }
}

void Compiler::statement() {
    addCode(SYMBOL_OP_LINE);
    addCode(engine->storage.makeNumber(tokenizer->getCurrent().line));
    expression();
}

std::pair<int, int> Compiler::findSymbol(QString name) {
    Word majorIndex = 1;
    while (majorIndex <= symbolTable.size()) {
        std::vector<QString>* symbols = symbolTable[majorIndex - 1];
        Word minorIndex = 1;
        while (minorIndex <= symbols->size()) {
            if (symbols->at(minorIndex - 1) == name) {
                return std::pair<int, int>(majorIndex, minorIndex);
            }
            minorIndex++;
        }
        majorIndex++;
    }
    return std::pair<int, int>(-1, -1);
}

void Compiler::expression() {
    if (tokenizer->isCurrent(TT_L_BRACE) &&
            tokenizer->isLookahead(TT_NAME) &&
            tokenizer->isLookahead2(TT_KOMMA))
    {
        definition();
    } else if (tokenizer->isCurrent(TT_NAME) &&
               tokenizer->isLookahead(TT_ARROW))
    {
        shortDefinition();
    } else if (tokenizer->isCurrent(TT_L_BRACKET)) {
        inlineDefinition();
    } else {
        basicExp();
    }
}

void Compiler::definition() {
    tokenizer->fetch(); // (
    std::vector<QString>* symbols = new std::vector<QString>();
    symbols->push_back(tokenizer->getCurrentString());
    tokenizer->fetch(); // 1st param
    while(tokenizer->isCurrent(TT_KOMMA)) {
        tokenizer->fetch(); // ,
        symbols->push_back(tokenizer->getCurrentString());
        tokenizer->fetch(); // nth param
    }
    expect(TT_R_BRACE, ")");
    expect(TT_ARROW, "->");
    symbolTable.insert(symbolTable.begin(), symbols);
    if (tokenizer->isCurrent(TT_L_CURLY)) {
        generateGuardedFunctionCode();
    } else {
        bool brackets = false;
        if (tokenizer->isCurrent(TT_L_BRACKET)) {
            tokenizer->fetch();
            brackets = true;
        }
        addCode(SYMBOL_OP_LDF);
        generateFunctionCode(brackets, true);
    }
    symbolTable.erase(symbolTable.begin());
    delete symbols;
}

void Compiler::generateGuardedFunctionCode() {
    expect(TT_L_CURLY, "{");
    addCode(SYMBOL_OP_LDF);
    Atom backupCode = code;
    Atom backupTail = tail;
    code = NIL;
    tail = NIL;
    do {
        expect(TT_L_BRACKET, "[");
        bool asSublist = false;
        if (!tokenizer->isCurrent(TT_COLON)) {
            asSublist = true;
            basicExp();
            addCode(SYMBOL_OP_BT);
        }
        expect(TT_COLON, ":");
        generateFunctionCode(true, asSublist);
    } while(tokenizer->isCurrent(TT_L_BRACKET));
    addCode(SYMBOL_OP_RTN);
    Atom fn = code;
    code = backupCode;
    tail = backupTail;
    addCode(fn);
    expect(TT_R_CURLY, "}");
}

void Compiler::shortDefinition() {
    std::vector<QString>* symbols = new std::vector<QString>();
    symbols->push_back(tokenizer->getCurrentString());
    tokenizer->fetch(); // param name...
    expect(TT_ARROW, "->");
    symbolTable.insert(symbolTable.begin(), symbols);
    if (tokenizer->isCurrent(TT_L_CURLY)) {
        generateGuardedFunctionCode();
    } else {
        bool brackets = false;
        if (tokenizer->isCurrent(TT_L_BRACKET)) {
            tokenizer->fetch();
            brackets = true;
        }
        addCode(SYMBOL_OP_LDF);
        generateFunctionCode(brackets, true);
    }
    symbolTable.erase(symbolTable.begin());
    delete symbols;
}

void Compiler::inlineDefinition() {
    tokenizer->fetch(); // [
    std::vector<QString>* symbols = new std::vector<QString>();
    if (tokenizer->isCurrent(TT_NAME) &&
            (tokenizer->isLookahead(TT_KOMMA) ||
             tokenizer->isLookahead(TT_ARROW)))
    {
        symbols->push_back(tokenizer->getCurrentString());
        tokenizer->fetch(); // 1st param
        while(tokenizer->isCurrent(TT_KOMMA)) {
            tokenizer->fetch(); // ,
            symbols->push_back(tokenizer->getCurrentString());
            tokenizer->fetch(); // nth param
        }
        expect(TT_ARROW, "->");
    } else if (tokenizer->isCurrent(TT_L_BRACE) &&
               (tokenizer->isLookahead(TT_NAME) &&
                tokenizer->isLookahead2(TT_KOMMA))) {
        tokenizer->fetch(); // (
        symbols->push_back(tokenizer->getCurrentString());
        tokenizer->fetch(); // 1st param
        while(tokenizer->isCurrent(TT_KOMMA)) {
            tokenizer->fetch(); // ,
            symbols->push_back(tokenizer->getCurrentString());
            tokenizer->fetch(); // nth param
        }
        expect(TT_R_BRACE, ")");
        expect(TT_ARROW, "->");
    } else if (tokenizer->isCurrent(TT_ARROW)) {
        tokenizer->fetch(); // ->
    }
    symbolTable.insert(symbolTable.begin(), symbols);
    addCode(SYMBOL_OP_LDF);
    generateFunctionCode(true, true);
    symbolTable.erase(symbolTable.begin());
    delete symbols;
}

void Compiler::generateFunctionCode(bool expectBracet, bool asSublist) {
    Atom backupCode = code;
    Atom backupTail = tail;
    if (asSublist) {
        code = NIL;
        tail = NIL;
    }
    addCode(SYMBOL_OP_FILE);
    addCode(file);
    if (expectBracet) {
        while (!tokenizer->isCurrent(TT_R_BRACKET) &&
               !tokenizer->isCurrent(TT_EOF))
        {
            block();
            if (!tokenizer->isCurrent(TT_R_BRACKET)) {
                addError(tokenizer->getCurrent(), "Missing Semicolon!");
            }
        }
        expect(TT_R_BRACKET, "]");
    } else {
        statement();
    }
    addCode(SYMBOL_OP_RTN);
    if (asSublist) {
        Atom fn = code;
        code = backupCode;
        tail = backupTail;
        addCode(fn);
    }
}


void Compiler::relExp() {
    termExp();
    Atom lastSubexpression = NIL;
    while(true) {
        Atom opCode = NIL;
        if (tokenizer->isCurrent(TT_EQ)) {
            opCode = SYMBOL_OP_EQ;
        } else if (tokenizer->isCurrent(TT_NE)) {
            opCode = SYMBOL_OP_NE;
        } else if (tokenizer->isCurrent(TT_LT)) {
            opCode = SYMBOL_OP_LT;
        } else if (tokenizer->isCurrent(TT_LTEQ)) {
            opCode = SYMBOL_OP_LTQ;
        } else if (tokenizer->isCurrent(TT_GT)) {
            opCode = SYMBOL_OP_GT;
        } else if (tokenizer->isCurrent(TT_GTEQ)) {
            opCode = SYMBOL_OP_GTQ;
        }
        if (opCode == NIL) {
            return;
        }
        tokenizer->fetch(); // Read over operator
        bool conjunction = lastSubexpression != NIL;
        if (conjunction) {
            // if we're in a conjunction, like 1 < x < 10, copy last
            // argument (x in this case) so we build an expression like
            // 1 < x & x < 10
            Atom code = lastSubexpression;
            Atom stop = tail;
            while(isCons(code) && code != stop) {
                Cons cell = engine->storage.getCons(code);
                addCode(cell->car);
                code = cell->cdr;
            }
        }
        lastSubexpression = tail;
        termExp();
        // Remember second argument in case we have a conjunction like
        // 1 < x < 10...
        lastSubexpression = engine->storage.getCons(lastSubexpression)->cdr;
        addCode(opCode);
        if (conjunction) {
            addCode(SYMBOL_OP_AND);
        }
    }
}

void Compiler::basicExp() {
    logExp();
    while(true) {
        if (tokenizer->isCurrent(TT_AND)) {
            tokenizer->fetch();
            logExp();
            addCode(SYMBOL_OP_AND);
        } else if (tokenizer->isCurrent(TT_OR)) {
            tokenizer->fetch();
            logExp();
            addCode(SYMBOL_OP_OR);
        } else {
            return;
        }
    }
}

void Compiler::logExp() {
    relExp();
    while(true) {
        if (tokenizer->isCurrent(TT_PLUS)) {
            tokenizer->fetch();
            relExp();
            addCode(SYMBOL_OP_ADD);
        } else if (tokenizer->isCurrent(TT_CONCAT)) {
            tokenizer->fetch();
            relExp();
            addCode(SYMBOL_OP_CONCAT);
        } else if (tokenizer->isCurrent(TT_MINUS)) {
            tokenizer->fetch();
            relExp();
            addCode(SYMBOL_OP_SUB);
        } else if (tokenizer->isCurrent(TT_NUMBER) &&
                   tokenizer->getCurrentString().toInt() < 0)
        {
            addCode(SYMBOL_OP_LDC);
            addCode(engine->storage.makeNumber(
                        tokenizer->getCurrentString().toInt()));
            addCode(SYMBOL_OP_ADD);
            tokenizer->fetch();
        } else {
            return;
        }
    }
}

void Compiler::termExp() {
    factorExp();
    while(true) {
        if (tokenizer->isCurrent(TT_MUL)) {
            tokenizer->fetch();
            factorExp();
            addCode(SYMBOL_OP_MUL);
        } else if (tokenizer->isCurrent(TT_DIV)) {
            tokenizer->fetch();
            factorExp();
            addCode(SYMBOL_OP_DIV);
        } else if (tokenizer->isCurrent(TT_MOD)) {
            tokenizer->fetch();
            factorExp();
            addCode(SYMBOL_OP_REM);
        } else {
            return;
        }
    }
}

void Compiler::factorExp() {
    if (tokenizer->isCurrent(TT_L_BRACE)) {
        tokenizer->fetch();
        expression();
        expect(TT_R_BRACE, ")");
    } else if (tokenizer->isCurrent(TT_NOT)) {
        tokenizer->fetch();
        factorExp();
        addCode(SYMBOL_OP_NOT);
    } else {
        if (tokenizer->isCurrent(TT_SYMBOL) ||
                tokenizer->isCurrent(TT_STRING) ||
                tokenizer->isCurrent(TT_DECIMAL) ||
                tokenizer->isCurrent(TT_NUMBER))
        {
            literal();
        } else if (tokenizer->isCurrent(TT_NAME)  &&
                   tokenizer->isLookahead(TT_SPLIT))
        {
            splitAssignment();
        } else if (tokenizer->isCurrent(TT_LIST_START)) {
            inlineList();
        } else if (tokenizer->isCurrent(TT_NAME)) {
            if (tokenizer->isLookahead(TT_L_BRACE)) {
                call();
            } else if (tokenizer->isLookahead(TT_ASSIGNMENT)) {
                localAssignment();
            } else if (tokenizer->isLookahead(TT_GLOBAL_ASSIGNMENT)) {
                globalAssignment();
            } else if (tokenizer->isLookahead(TT_SPLIT)) {
                splitAssignment();
            } else if(tokenizer->getCurrentString().endsWith(':')
                      && !tokenizer->isLookahead(TT_R_BRACE)
                      && !tokenizer->isLookahead(TT_R_BRACKET)
                      && !tokenizer->isLookahead(TT_KOMMA)
                      && !tokenizer->isLookahead(TT_EOF)
                      && !tokenizer->isLookahead(TT_SEMICOLON)) {
                call();
            } else {
                variable();
            }
        } else {
            addError(tokenizer->getCurrent(),
                     QString("Unexpected token: ") +
                     tokenizer->getCurrentString());
            tokenizer->fetch();
        }
    }
}

void Compiler::inlineList() {
    tokenizer->fetch(); // #(
    addCode(SYMBOL_OP_NIL);
    bool useChain = false;
    while(!tokenizer->isCurrent(TT_R_BRACE) && !tokenizer->isCurrent(TT_EOF)) {
        expression();
        addCode(SYMBOL_OP_CHAIN);
        useChain = true;
        if (tokenizer->isCurrent(TT_KOMMA)) {
            tokenizer->fetch();
        }
    }
    if (useChain) {
        addCode(SYMBOL_OP_CHAIN_END);
    }
    expect(TT_R_BRACE, ")");
}

Atom Compiler::compileLiteral() {
    Atom result = NIL;
    if (tokenizer->isCurrent(TT_SYMBOL)) {
         result = engine->storage.makeSymbol(
                 tokenizer->getCurrentString().
                 mid(1, tokenizer->getCurrent().length - 1));
    } else if (tokenizer->isCurrent(TT_STRING)) {
        result = engine->storage.makeString(
                tokenizer->getCurrentString().
                mid(1, tokenizer->getCurrent().length - 2));
    } else if (tokenizer->isCurrent(TT_NUMBER)) {
        result = engine->storage.makeNumber(
                tokenizer->getCurrentString().toInt());
    } else if (tokenizer->isCurrent(TT_DECIMAL)) {
        result = engine->storage.makeDecimal(
                tokenizer->getCurrentString().toDouble());
    } else {
        addError(tokenizer->getCurrent(), "Unexpected token! Expected a literal");
    }
    tokenizer->fetch();
    return result;
}

void Compiler::literal() {
    addCode(SYMBOL_OP_LDC);
    addCode(compileLiteral());
}

void Compiler::variable() {
    QString name = tokenizer->getCurrentString();
    tokenizer->fetch();
    load(name);
}

void Compiler::load(QString variable) {
    std::pair<int, int> pair = findSymbol(variable);
    if (pair.first > 0) {
        addCode(SYMBOL_OP_LD);
        addCode(engine->storage.makeCons(engine->storage.makeNumber(pair.first), engine->storage.makeNumber(pair.second)));
    } else {
        Atom symbol = engine->storage.makeSymbol(variable);
        Atom bif = engine->findBuiltInFunction(symbol);
        if (bif != NIL){
            addCode(SYMBOL_OP_LDC);
            addCode(bif);
        } else {
            addCode(SYMBOL_OP_LDG);
            addCode(engine->storage.findGlobal(symbol));
        }
    }
}

void Compiler::call() {
    if (tokenizer->getCurrentString().endsWith(':')) {
        colonCall();
    } else {
        standardCall();
    }
}

void Compiler::colonCall() {
    QString name("");
    addCode(SYMBOL_OP_NIL);
    while(tokenizer->isCurrent(TT_NAME) &&
          tokenizer->getCurrentString().endsWith(':')) {
        name += tokenizer->getCurrentString();
        tokenizer->fetch();
        expression();
        addCode(SYMBOL_OP_CHAIN);
    }
    addCode(SYMBOL_OP_CHAIN_END);
    load(name);
    addCode(SYMBOL_OP_AP);
}

void Compiler::standardCall() {
    QString name = tokenizer->getCurrentString();
    tokenizer->fetch(); // name
    tokenizer->fetch(); // (
    if (!tokenizer->isCurrent(TT_R_BRACE)) {
        addCode(SYMBOL_OP_NIL);
        while(!tokenizer->isCurrent(TT_R_BRACE) &&
              !tokenizer->isCurrent(TT_EOF))
        {
            expression();
            addCode(SYMBOL_OP_CHAIN);
            if (tokenizer->isCurrent(TT_KOMMA)) {
                tokenizer->fetch();
            }
        }
        addCode(SYMBOL_OP_CHAIN_END);
        expect(TT_R_BRACE, ")");
        load(name);
        addCode(SYMBOL_OP_AP);
    } else {
        expect(TT_R_BRACE, ")");
        load(name);
        addCode(SYMBOL_OP_AP0);
    }
}

void Compiler::splitAssignment() {
    if (symbolTable.size() == 0) {
        addError(tokenizer->getCurrent(),
                 "Split-Assignments not allowed on top-level");
        tokenizer->fetch(); // name
        tokenizer->fetch(); // |
        tokenizer->fetch(); // name
        expect(TT_ASSIGNMENT, ":=");
        return;
    }
    QString headName = tokenizer->getCurrentString();
    tokenizer->fetch(); // name
    tokenizer->fetch(); // |
    QString tailName = tokenizer->getCurrentString();
    tokenizer->fetch(); // name
    expect(TT_ASSIGNMENT, ":=");
    std::vector<QString>* symbols = symbolTable[0];
    Word headMinorIndex = 1;
    while (headMinorIndex <= symbols->size()) {
        if (symbols->at(headMinorIndex - 1) == headName) {
            break;
        }
        headMinorIndex++;
    }
    if (headMinorIndex > symbols->size()) {
        symbols->push_back(headName);
    }
    Word tailMinorIndex = 1;
    while (tailMinorIndex <= symbols->size()) {
        if (symbols->at(tailMinorIndex - 1) == tailName) {
            break;
        }
        tailMinorIndex++;
    }
    if (tailMinorIndex > symbols->size()) {
        symbols->push_back(tailName);
    }
    expression();
    addCode(SYMBOL_OP_SPLIT);
    addCode(engine->storage.makeCons(engine->storage.makeNumber(1), engine->storage.makeNumber(headMinorIndex)));
    addCode(engine->storage.makeCons(engine->storage.makeNumber(1), engine->storage.makeNumber(tailMinorIndex)));
}

void Compiler::localAssignment() {
    if (symbolTable.size() == 0) {
        globalAssignment();
        return;
    }
    QString name = tokenizer->getCurrentString();
    tokenizer->fetch(); // name
    tokenizer->fetch(); // :=
    std::vector<QString>* symbols = symbolTable[0];
    Word minorIndex = 1;
    while (minorIndex <= symbols->size()) {
        if (symbols->at(minorIndex - 1) == name) {
            break;
        }
        minorIndex++;
    }
    if (minorIndex > symbols->size()) {
        symbols->push_back(name);
    }
    expression();
    addCode(SYMBOL_OP_ST);
    addCode(engine->storage.makeCons(engine->storage.makeNumber(1), engine->storage.makeNumber(minorIndex)));
}

void Compiler::globalAssignment() {
    QString name = tokenizer->getCurrentString();
    tokenizer->fetch(); // name
    tokenizer->fetch(); // ::=
    expression();
    std::pair<int, int> pair = findSymbol(name);
    if (pair.first > 0) {
        addCode(SYMBOL_OP_ST);
        addCode(engine->storage.makeCons(
                    engine->storage.makeNumber(pair.first),
                    engine->storage.makeNumber(pair.second)));
    } else {
        addCode(SYMBOL_OP_STG);
        addCode(engine->storage.findGlobal(engine->storage.makeSymbol(name)));
    }
}
