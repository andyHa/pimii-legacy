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

#include "compiler.h"

Compiler::Compiler(const QString& fileName,
                   const QString& input,
                   Engine* engine) :
    engine(engine),
    tokenizer(input, true),
    code(engine->storage.ref(NIL)),
    tail(engine->storage.ref(NIL))
{
    lastLine = 0;
    file = engine->storage.ref(engine->storage.makeSymbol(fileName));
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
    if (tokenizer.getCurrent().type != tt) {
        addError(tokenizer.getCurrent(),
                 QString("Unexpected token: ") +
                 tokenizer.getCurrentString() +
                 QString(". Expected: ")
                 +QString(rep));
    } else {
        tokenizer.fetch();
    }
}

void Compiler::addCode(Atom atom) {
    if (isNil(code->atom())) {
        code->atom(engine->storage.makeCons(atom, NIL));
        tail->atom(code->atom());
    } else {
        tail->atom(engine->storage.append(tail->atom(), atom));
    }
}

std::vector<CompilationError> Compiler::getErrors() {
    return errors;
}

Atom Compiler::getCode() {
    return code->atom();
}

bool Compiler::compile(bool appendStop) {
    code->atom(NIL);
    inCondtitional = false;
    errors.clear();
    tokenizer.fetch();
    updatePosition(true);
    while(tokenizer.getCurrent().type != TT_EOF) {
        block();
        if (tokenizer.getCurrent().type != TT_EOF) {
            addError(tokenizer.getCurrent(), "Missing Semicolon!");
        }
    }
    if (appendStop) {
        addCode(SYMBOL_OP_STOP);
    } else {
        addCode(SYMBOL_OP_RTN);
    }
    return errors.empty();
}

void Compiler::block() {
    statement();
    while(!tokenizer.isCurrent(TT_R_CURLY) && !tokenizer.isCurrent(TT_R_BRACKET) && !tokenizer.isCurrent(TT_EOF)) {
        if (tokenizer.isCurrent(TT_SEMICOLON)) {
            tokenizer.fetch();
        } else {
            statement();
        }
    }
}

void Compiler::updatePosition(bool force) {
    if (force) {
        addCode(SYMBOL_OP_FILE);
        addCode(file->atom());
        addCode(SYMBOL_OP_LINE);
        addCode(engine->storage.makeNumber(tokenizer.getCurrent().line));
    } else if (lastLine != tokenizer.getCurrent().line) {
        addCode(SYMBOL_OP_LINE);
        addCode(engine->storage.makeNumber(tokenizer.getCurrent().line));
    }
    lastLine = tokenizer.getCurrent().line;
}

void Compiler::statement() {
    updatePosition(false);
    expression();
}

EnvPos Compiler::findSymbol(QString name) {
    Word majorIndex = 1;
    while (majorIndex <= symbolTable.size()) {
        std::vector<QString>* symbols = symbolTable[majorIndex - 1];
        Word minorIndex = 1;
        while (minorIndex <= symbols->size()) {
            if (symbols->at(minorIndex - 1) == name) {
                return EnvPos(majorIndex, minorIndex);
            }
            minorIndex++;
        }
        majorIndex++;
    }
    return EnvPos(-1 , -1);
}

void Compiler::expression() {
    if (tokenizer.isCurrent(TT_L_BRACE) &&
            tokenizer.isLookahead(TT_NAME) &&
            tokenizer.isLookahead2(TT_KOMMA))
    {
        normalDefinition();
    } else if (tokenizer.isCurrent(TT_NAME) &&
               tokenizer.isLookahead(TT_ARROW))
    {
        shortDefinition();
    } else if (tokenizer.isCurrent(TT_L_BRACKET))
    {
        conditional();
    } else if (tokenizer.isCurrent(TT_L_CURLY))
    {
        inlineDefinition();
    } else {
        basicExp();
    }
}

void Compiler::normalDefinition() {
    tokenizer.fetch(); // (
    std::vector<QString>* symbols = new std::vector<QString>();
    symbols->push_back(tokenizer.getCurrentString());
    tokenizer.fetch(); // 1st param
    while(tokenizer.isCurrent(TT_KOMMA)) {
        tokenizer.fetch(); // ,
        symbols->push_back(tokenizer.getCurrentString());
        tokenizer.fetch(); // nth param
    }
    expect(TT_R_BRACE, ")");
    expect(TT_ARROW, "->");
    symbolTable.insert(symbolTable.begin(), symbols);
    bool brackets = false;
    if (tokenizer.isCurrent(TT_L_CURLY)) {
        tokenizer.fetch();
        brackets = true;
    }
    addCode(SYMBOL_OP_LDF);
    generateFunctionCode(brackets, true);
    symbolTable.erase(symbolTable.begin());
    delete symbols;
}

void Compiler::conditional() {
    expect(TT_L_BRACKET, "[");
    updatePosition(false);
    basicExp();
    addCode(SYMBOL_OP_BT);
    expect(TT_COLON, ":");
    AtomRef* backupCode = engine->storage.ref(code->atom());
    AtomRef* backupTail = engine->storage.ref(tail->atom());
    bool backupInConditional = inCondtitional;
    inCondtitional = true;
    code->atom(NIL);
    tail->atom(NIL);
    block();
    addCode(SYMBOL_OP_JOIN);
    if (!tokenizer.isCurrent(TT_R_BRACKET)) {
        addError(tokenizer.getCurrent(), "Missing Semicolon!");
    }
    expect(TT_R_BRACKET, "]");
    Atom fn = code->atom();
    code->atom(backupCode->atom());
    delete backupCode;
    tail->atom(backupTail->atom());
    delete backupTail;
    inCondtitional = backupInConditional;
    addCode(fn);
}

void Compiler::shortDefinition() {
    std::vector<QString>* symbols = new std::vector<QString>();
    symbols->push_back(tokenizer.getCurrentString());
    tokenizer.fetch(); // param name...
    expect(TT_ARROW, "->");
    symbolTable.insert(symbolTable.begin(), symbols);
    bool brackets = false;
    if (tokenizer.isCurrent(TT_L_CURLY)) {
        tokenizer.fetch();
        brackets = true;
    }
    addCode(SYMBOL_OP_LDF);
    generateFunctionCode(brackets, true);
    symbolTable.erase(symbolTable.begin());
    delete symbols;
}

void Compiler::inlineDefinition() {
    tokenizer.fetch(); // {
    addCode(SYMBOL_OP_LDF);
    generateFunctionCode(true, true);
}

void Compiler::generateFunctionCode(bool expectBracet, bool asSublist) {
    AtomRef* backupCode = engine->storage.ref(code->atom());
    AtomRef* backupTail = engine->storage.ref(tail->atom());
    bool backupInConditional = inCondtitional;
    if (asSublist) {
        code->atom(NIL);
        tail->atom(NIL);
        inCondtitional = false;
    }
    updatePosition(true);
    if (expectBracet) {
        while (!tokenizer.isCurrent(TT_R_CURLY) &&
               !tokenizer.isCurrent(TT_EOF))
        {
            block();
            if (!tokenizer.isCurrent(TT_R_CURLY)) {
                addError(tokenizer.getCurrent(), "Missing Semicolon!");
            }
        }
        expect(TT_R_CURLY, "}");
    } else {
        statement();
    }
    addCode(SYMBOL_OP_RTN);
    if (asSublist) {
        Atom fn = code->atom();
        code->atom(backupCode->atom());
        delete backupCode;
        tail->atom(backupTail->atom());
        delete backupTail;
        addCode(fn);
        inCondtitional = backupInConditional;
    }
}


void Compiler::relExp() {
    termExp();
    Atom lastSubexpressionStart = NIL;
    Atom lastSubexpressionEnd = NIL;
    while(true) {
        Atom opCode = NIL;
        if (tokenizer.isCurrent(TT_EQ)) {
            opCode = SYMBOL_OP_EQ;
        } else if (tokenizer.isCurrent(TT_NE)) {
            opCode = SYMBOL_OP_NE;
        } else if (tokenizer.isCurrent(TT_LT)) {
            opCode = SYMBOL_OP_LT;
        } else if (tokenizer.isCurrent(TT_LTEQ)) {
            opCode = SYMBOL_OP_LTQ;
        } else if (tokenizer.isCurrent(TT_GT)) {
            opCode = SYMBOL_OP_GT;
        } else if (tokenizer.isCurrent(TT_GTEQ)) {
            opCode = SYMBOL_OP_GTQ;
        }
        if (opCode == NIL) {
            return;
        }
        tokenizer.fetch(); // Read over operator
        bool conjunction = lastSubexpressionStart != NIL;
        if (conjunction) {
            // if we're in a conjunction, like 1 < x < 10, copy last
            // argument (x in this case) so we build an expression like
            // 1 < x & x < 10
            Atom code = engine->storage.getCons(lastSubexpressionStart).cdr;
            while(isCons(code) && code != lastSubexpressionEnd) {
                Cell cell = engine->storage.getCons(code);
                addCode(cell.car);
                code = cell.cdr;
            }
        }
        lastSubexpressionStart = tail->atom();
        termExp();
        // Remember second argument in case we have a conjunction like
        // 1 < x < 10...
        addCode(opCode);
        lastSubexpressionEnd = tail->atom();
        if (conjunction) {
            addCode(SYMBOL_OP_AND);
        }
    }
}

void Compiler::basicExp() {
    logExp();
    while(true) {
        if (tokenizer.isCurrent(TT_AND)) {
            tokenizer.fetch();
            logExp();
            addCode(SYMBOL_OP_AND);
        } else if (tokenizer.isCurrent(TT_OR)) {
            tokenizer.fetch();
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
        if (tokenizer.isCurrent(TT_PLUS)) {
            tokenizer.fetch();
            relExp();
            addCode(SYMBOL_OP_ADD);
        } else if (tokenizer.isCurrent(TT_CONCAT)) {
            tokenizer.fetch();
            relExp();
            addCode(SYMBOL_OP_CONCAT);
        } else if (tokenizer.isCurrent(TT_MINUS)) {
            tokenizer.fetch();
            relExp();
            addCode(SYMBOL_OP_SUB);
        } else if (tokenizer.isCurrent(TT_NUMBER) &&
                   tokenizer.getCurrentString().toInt() < 0)
        {
            addCode(SYMBOL_OP_LDC);
            addCode(engine->storage.makeNumber(
                        tokenizer.getCurrentString().toInt()));
            addCode(SYMBOL_OP_ADD);
            tokenizer.fetch();
        } else {
            return;
        }
    }
}

void Compiler::termExp() {
    factorExp();
    while(true) {
        if (tokenizer.isCurrent(TT_MUL)) {
            tokenizer.fetch();
            factorExp();
            addCode(SYMBOL_OP_MUL);
        } else if (tokenizer.isCurrent(TT_DIV)) {
            tokenizer.fetch();
            factorExp();
            addCode(SYMBOL_OP_DIV);
        } else if (tokenizer.isCurrent(TT_MOD)) {
            tokenizer.fetch();
            factorExp();
            addCode(SYMBOL_OP_REM);
        } else {
            return;
        }
    }
}

void Compiler::factorExp() {
    if (tokenizer.isCurrent(TT_L_BRACE)) {
        tokenizer.fetch();
        expression();
        expect(TT_R_BRACE, ")");
    } else if (tokenizer.isCurrent(TT_NOT)) {
        tokenizer.fetch();
        factorExp();
        addCode(SYMBOL_OP_NOT);
    } else {
        if (tokenizer.isCurrent(TT_SYMBOL) ||
                tokenizer.isCurrent(TT_STRING) ||
                tokenizer.isCurrent(TT_DECIMAL) ||
                tokenizer.isCurrent(TT_NUMBER))
        {
            literal();
        } else if (tokenizer.isCurrent(TT_NAME)  &&
                   tokenizer.isLookahead(TT_SPLIT))
        {
            splitAssignment();
        } else if (tokenizer.isCurrent(TT_LIST_START)) {
            inlineList();
        } else if (tokenizer.isCurrent(TT_RTN)) {
            rtn();
        } else if (tokenizer.isCurrent(TT_NAME)) {
            if (tokenizer.isLookahead(TT_L_BRACE)) {
                call();
            } else if (tokenizer.isLookahead(TT_ASSIGNMENT)) {
                localAssignment();
            } else if (tokenizer.isLookahead(TT_GLOBAL_ASSIGNMENT)) {
                globalAssignment();
            } else if (tokenizer.isLookahead(TT_SPLIT)) {
                splitAssignment();
            } else if(tokenizer.getCurrentString().endsWith(':')
                      && !tokenizer.isLookahead(TT_R_BRACE)
                      && !tokenizer.isLookahead(TT_R_BRACKET)
                      && !tokenizer.isLookahead(TT_KOMMA)
                      && !tokenizer.isLookahead(TT_EOF)
                      && !tokenizer.isLookahead(TT_SEMICOLON))
            {
                call();
            } else {
                variable();
            }
        } else {
            addError(tokenizer.getCurrent(),
                     QString("Unexpected token: ") +
                     tokenizer.getCurrentString());
            tokenizer.fetch();
        }
    }
}

void Compiler::inlineList() {
    tokenizer.fetch(); // #(
    if (tokenizer.isCurrent(TT_R_BRACE)) {
        // #() is nil
        tokenizer.fetch(); // )
        addCode(SYMBOL_OP_NIL);
        return;
    }

    if (tokenizer.isLookahead(TT_DOT)) {
        Atom car = compileLiteral();
        tokenizer.fetch(); // .
        Atom cdr = compileLiteral();
        addCode(SYMBOL_OP_LDC);
        addCode(engine->storage.makeCons(car, cdr));
    } else {
        addCode(SYMBOL_OP_NIL);
        while(!tokenizer.isCurrent(TT_R_BRACE) &&
              !tokenizer.isCurrent(TT_EOF))
        {
            expression();
            addCode(SYMBOL_OP_CONS);
            if (tokenizer.isCurrent(TT_KOMMA)) {
                tokenizer.fetch();
            }
        }
    }
    expect(TT_R_BRACE, ")");
}

Atom Compiler::compileLiteral() {
    Atom result = NIL;
    if (tokenizer.isCurrent(TT_SYMBOL)) {
         result = engine->storage.makeSymbol(
                 tokenizer.getCurrentString().
                 mid(1, tokenizer.getCurrent().length - 1));
    } else if (tokenizer.isCurrent(TT_STRING)) {
        result = engine->storage.makeString(
                tokenizer.getCurrentString().
                mid(1, tokenizer.getCurrent().length - 2));
    } else if (tokenizer.isCurrent(TT_NUMBER)) {
        result = engine->storage.makeNumber(
                tokenizer.getCurrentString().toInt());
    } else if (tokenizer.isCurrent(TT_DECIMAL)) {
        result = engine->storage.makeDecimal(
                tokenizer.getCurrentString().toDouble());
    } else {
        addError(tokenizer.getCurrent(),
                 "Unexpected token! Expected a literal");
    }
    tokenizer.fetch();
    return result;
}

void Compiler::literal() {
    addCode(SYMBOL_OP_LDC);
    addCode(compileLiteral());
}

void Compiler::variable() {
    QString name = tokenizer.getCurrentString();
    tokenizer.fetch();
    load(name);
}

void Compiler::load(QString variable) {
    EnvPos pos = findSymbol(variable);
    if (pos.major > 0) {
        addCode(SYMBOL_OP_LD);
        addCode(engine->storage.makeCons(
                    engine->storage.makeNumber(pos.major),
                    engine->storage.makeNumber(pos.minor)));
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
    if (tokenizer.getCurrentString().endsWith(':')) {
        colonCall();
    } else {
        standardCall();
    }
}

void Compiler::colonCall() {
    QString name("");
    addCode(SYMBOL_OP_NIL);
    AtomRef* backupCode = engine->storage.ref(code->atom());
    AtomRef* backupTail = engine->storage.ref(tail->atom());
    AtomRef* argsCode = engine->storage.ref(NIL);
    AtomRef* argsTail = engine->storage.ref(NIL);

    while(tokenizer.isCurrent(TT_NAME) &&
          tokenizer.getCurrentString().endsWith(':')) {
        name += tokenizer.getCurrentString();
        tokenizer.fetch();
        code->atom(NIL);
        tail->atom(NIL);
        expression();
        addCode(SYMBOL_OP_CONS);
        if (isNil(argsCode->atom())) {
            argsCode->atom(code->atom());
            argsTail->atom(tail->atom());
        } else {
            engine->storage.setCDR(tail->atom(), argsCode->atom());
            argsCode->atom(code->atom());
        }
    }
    code->atom(backupCode->atom());
    engine->storage.setCDR(backupTail->atom(), argsCode->atom());
    tail->atom(argsTail->atom());
    delete backupCode;
    delete backupTail;
    delete argsCode;
    delete argsTail;
    load(name);
    addCode(SYMBOL_OP_AP);
    addCode(engine->storage.makeSymbol(name));
}

void Compiler::standardCall() {    
    QString name = tokenizer.getCurrentString();
    tokenizer.fetch(); // name
    tokenizer.fetch(); // (
    if (!tokenizer.isCurrent(TT_R_BRACE)) {
        addCode(SYMBOL_OP_NIL);
        AtomRef* backupCode = engine->storage.ref(code->atom());
        AtomRef* backupTail = engine->storage.ref(tail->atom());
        AtomRef* argsCode = engine->storage.ref(NIL);
        AtomRef* argsTail = engine->storage.ref(NIL);

        while(!tokenizer.isCurrent(TT_R_BRACE) &&
              !tokenizer.isCurrent(TT_EOF))
        {
            code->atom(NIL);
            tail->atom(NIL);
            expression();
            addCode(SYMBOL_OP_CONS);
            if (isNil(argsCode->atom())) {
                argsCode->atom(code->atom());
                argsTail->atom(tail->atom());
            } else {
                engine->storage.setCDR(tail->atom(), argsCode->atom());
                argsCode->atom(code->atom());
            }
            if (tokenizer.isCurrent(TT_KOMMA)) {
                tokenizer.fetch();
            }
        }

        expect(TT_R_BRACE, ")");
        code->atom(backupCode->atom());
        engine->storage.setCDR(backupTail->atom(), argsCode->atom());
        tail->atom(argsTail->atom());
        delete backupCode;
        delete backupTail;
        delete argsCode;
        delete argsTail;
        load(name);
        addCode(SYMBOL_OP_AP);
        addCode(engine->storage.makeSymbol(name));

    } else {
        expect(TT_R_BRACE, ")");
        load(name);
        addCode(SYMBOL_OP_AP0);
        addCode(engine->storage.makeSymbol(name));
    }
}

void Compiler::splitAssignment() {
    if (symbolTable.size() == 0) {
        addError(tokenizer.getCurrent(),
                 "Split-Assignments not allowed on top-level");
        tokenizer.fetch(); // name
        tokenizer.fetch(); // |
        tokenizer.fetch(); // name
        expect(TT_ASSIGNMENT, ":=");
        return;
    }
    QString headName = tokenizer.getCurrentString();
    tokenizer.fetch(); // name
    tokenizer.fetch(); // |
    QString tailName = tokenizer.getCurrentString();
    tokenizer.fetch(); // name
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
    factorExp();
    addCode(SYMBOL_OP_SPLIT);
    addCode(engine->storage.makeCons(
                engine->storage.makeNumber(1),
                engine->storage.makeNumber(headMinorIndex)));
    addCode(engine->storage.makeCons(
                engine->storage.makeNumber(1),
                engine->storage.makeNumber(tailMinorIndex)));
}



void Compiler::rtn() {
    expect(TT_RTN, "^");
    expression();
    if (inCondtitional) {
        addCode(SYMBOL_OP_LONG_RTN);
    } else {
        addCode(SYMBOL_OP_RTN);
    }
}


void Compiler::localAssignment() {
    if (symbolTable.size() == 0) {
        globalAssignment();
        return;
    }
    QString name = tokenizer.getCurrentString();
    tokenizer.fetch(); // name
    tokenizer.fetch(); // :=
    EnvPos pos = findSymbol(name);
    Word majorIndex = 1;
    Word minorIndex = 1;
    if (pos.major == -1) {
        std::vector<QString>* symbols = symbolTable[0];
        while (minorIndex <= symbols->size()) {
            if (symbols->at(minorIndex - 1) == name) {
                break;
            }
            minorIndex++;
        }
        if (minorIndex > symbols->size()) {
            symbols->push_back(name);
        }
    } else {
        majorIndex = pos.major;
        minorIndex = pos.minor;
    }
    expression();
    addCode(SYMBOL_OP_ST);
    addCode(engine->storage.makeCons(
                engine->storage.makeNumber(majorIndex),
                engine->storage.makeNumber(minorIndex)));
}

void Compiler::globalAssignment() {
    QString name = tokenizer.getCurrentString();
    tokenizer.fetch(); // name
    tokenizer.fetch(); // ::=
    expression();
    EnvPos pos = findSymbol(name);
    if (pos.major > 0) {
        addCode(SYMBOL_OP_ST);
        addCode(engine->storage.makeCons(
                    engine->storage.makeNumber(pos.major),
                    engine->storage.makeNumber(pos.minor)));
    } else {
        addCode(SYMBOL_OP_STG);
        addCode(engine->storage.findGlobal(engine->storage.makeSymbol(name)));
    }
}
