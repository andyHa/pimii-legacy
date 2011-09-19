#include "parser.h"

Parser::Parser(Tokenizer& tokenizer, Visitor& visitor) :
    tokenizer(tokenizer),
    visitor(visitor)
{
}


void Parser::parse() {
    visitor.setParser(this);
    tokenizer.fetch();
    while(tokenizer.getCurrent().type != TT_EOF) {
        block();
        if (tokenizer.getCurrent().type != TT_EOF) {
            visitor.addError(tokenizer.getCurrent().line,
                             tokenizer.getCurrent().pos,
                             String(L"Missing Semicolon!"));
        }
    }
}

void Parser::block() {
    visitor.blockBegin();
    statement();
    while(tokenizer.getCurrent().type == TT_SEMICOLON
          && tokenizer.getLookahead().type != TT_R_BRACE
          && tokenizer.getLookahead().type != TT_R_BRACKET
          && tokenizer.getLookahead().type != TT_EOF) {
        tokenizer.fetch();
        statement();
    }
    if (tokenizer.getCurrent().type == TT_SEMICOLON) {
        tokenizer.fetch();
    }
    visitor.blockEnd();
}

void Parser::statement() {
    visitor.statementBegin();
    if (tokenizer.getCurrent().type == TT_NAME) {
        if (tokenizer.getLookahead().type == TT_ASSIGNMENT) {
            localAssignment();
            return;
        } else if (tokenizer.getLookahead().type == TT_GLOBAL_ASSIGNMENT) {
            globalAssignment();
            return;
        }
    }
    expression();
    visitor.statementEnd();
}

void Parser::localAssignment() {
    if (symbolTable.size() == 0) {
        globalAssignment();
        return;
    }
    String name = tokenizer.getCurrent().tokenString;
    tokenizer.fetch(); // name
    tokenizer.fetch(); // :=
    std::vector<String>* symbols = symbolTable[0];
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
    visitor.localAssignmentBegin(name, 1, minorIndex);
    expression();
    visitor.localAssignmentEnd(name, 1, minorIndex);
}

void Parser::globalAssignment() {
    String name = tokenizer.getCurrent().tokenString;
    tokenizer.fetch(); // name
    tokenizer.fetch(); // ::=
    std::pair<int, int> pair = findSymbol(name);
    visitor.globalAssignmentBegin(name, pair.first, pair.second);
    expression();
    visitor.globalAssignmentEnd(name, pair.first, pair.second);
}

std::pair<int, int> Parser::findSymbol(String name) {
    Word majorIndex = 1;
    while (majorIndex <= symbolTable.size()) {
        std::vector<String>* symbols = symbolTable[majorIndex - 1];
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

void Parser::expression() {
    visitor.expressionBegin();
    if (tokenizer.getCurrent().type == TT_L_BRACE
            && tokenizer.getLookahead().type == TT_NAME
            && tokenizer.getLookahead2().type == TT_KOMMA) {
        definition();
    } else if (tokenizer.getCurrent().type == TT_NAME
               && tokenizer.getLookahead().type == TT_ARROW) {
        shortDefinition();
    } else if (tokenizer.getCurrent().type == TT_L_BRACKET) {
        inlineDefinition();
    } else if (tokenizer.getCurrent().type == TT_ASM_BEGIN) {
        includeAsm();
    } else {
        basicExp();
    }
    visitor.expressionEnd();
}

void Parser::includeAsm() {
    visitor.includeAsmBegin();
    tokenizer.fetch();
    while(tokenizer.getCurrent().type != TT_ASM_END) {
        if (tokenizer.getCurrent().type == TT_L_BRACE) {
            handleAsmSublist();
        } else {
            visitor.onAsmLiteral();
        }
    }
    expect(TT_ASM_END, String(L">>"));
    visitor.includeAsmEnd();
}

void Parser::handleAsmSublist() {
    tokenizer.fetch(); // (
    if (tokenizer.getLookahead().type == TT_DOT) {
        Token left = tokenizer.getCurrent();
        tokenizer.fetch(); //.
        Token right = tokenizer.getCurrent();
        visitor.asmCreateTuple(left, right);
    } else {
        visitor.asmSublistBegin();
        while(tokenizer.getCurrent().type != TT_R_BRACE) {
            if (tokenizer.getCurrent().type == TT_L_BRACE) {
                handleAsmSublist();
            } else {
                visitor.onAsmLiteral();
            }
        }
        visitor.asmSublistEnd();
    }
    expect(TT_R_BRACE,String(L")"));
}

void Parser::definition() {
    visitor.definitionBegin(NORMAL_DEFINITION);
    tokenizer.fetch(); // (
    std::vector<String>* symbols = new std::vector<String>();
    symbols->push_back(tokenizer.getCurrent().tokenString);
    visitor.definitionAddParameter(tokenizer.getCurrent().tokenString);
    tokenizer.fetch(); // 1st param
    while(tokenizer.getCurrent().type == TT_KOMMA) {
        tokenizer.fetch(); // ,
        symbols->push_back(tokenizer.getCurrent().tokenString);
        visitor.definitionAddParameter(tokenizer.getCurrent().tokenString);
        tokenizer.fetch(); // nth param
    }
    expect(TT_R_BRACE, String(L")"));
    expect(TT_ARROW, String(L"->"));
    symbolTable.insert(symbolTable.begin(), symbols);
    if (tokenizer.getCurrent().type == TT_L_CURLY) {
        generateGuardedFunctionCode();
    } else {
        bool brackets = false;
        if (tokenizer.getCurrent().type == TT_L_BRACKET) {
            tokenizer.fetch();
            brackets = true;
        }
        visitor.definitionBodyBegin(brackets);
        generateFunctionCode(brackets);
        visitor.definitionBodyEnd(brackets);
    }
    symbolTable.erase(symbolTable.begin());
    delete symbols;
    visitor.definitionEnd(NORMAL_DEFINITION);
}

void Parser::generateGuardedFunctionCode() {
    visitor.guardedDefinitionBegin();
    expect(TT_L_CURLY, String(L"{"));
    do {
        visitor.guardedBranchBegin();
        expect(TT_L_BRACKET, String(L"["));
        bool isGuarded = false;
        if (tokenizer.getCurrent().type != TT_COLON) {
            visitor.guardBegin();
            isGuarded = true;
            basicExp();
            visitor.guardEnd();
        }
        expect(TT_COLON, String(L":"));
        visitor.guardedBlockBegin(isGuarded);
        generateFunctionCode(true);
        visitor.guardedBlockEnd();
    } while(tokenizer.getCurrent().type == TT_L_BRACKET);
    visitor.guardedDefinitionEnd();
    expect(TT_R_CURLY, String(L"}"));    
}

void Parser::shortDefinition() {
    visitor.definitionBegin(SHORT_DEFINITION);
    std::vector<String>* symbols = new std::vector<String>();
    symbols->push_back(tokenizer.getCurrent().tokenString);
    visitor.definitionAddParameter(tokenizer.getCurrent().tokenString);
    tokenizer.fetch(); // param name...
    expect(TT_ARROW, String(L"->"));
    symbolTable.insert(symbolTable.begin(), symbols);
    if (tokenizer.getCurrent().type == TT_L_CURLY) {
        generateGuardedFunctionCode();
    } else {
        bool brackets = false;
        if (tokenizer.getCurrent().type == TT_L_BRACKET) {
            tokenizer.fetch();
            brackets = true;
        }
        visitor.definitionBodyBegin(brackets);
        generateFunctionCode(brackets);
        visitor.definitionBodyEnd(brackets);
    }
    symbolTable.erase(symbolTable.begin());
    delete symbols;
    visitor.definitionEnd(SHORT_DEFINITION);
}

void Parser::inlineDefinition() {
    visitor.definitionBegin(INLINE_DEFINITION);
    tokenizer.fetch(); // [
    std::vector<String>* symbols = new std::vector<String>();
    if (tokenizer.getCurrent().type == TT_NAME &&
            (tokenizer.getLookahead().type == TT_KOMMA ||
             tokenizer.getLookahead().type == TT_ARROW))
    {
        symbols->push_back(tokenizer.getCurrent().tokenString);
        visitor.definitionAddParameter(tokenizer.getCurrent().tokenString);
        tokenizer.fetch(); // 1st param
        while(tokenizer.getCurrent().type == TT_KOMMA) {
            tokenizer.fetch(); // ,
            symbols->push_back(tokenizer.getCurrent().tokenString);
            visitor.definitionAddParameter(tokenizer.getCurrent().tokenString);
            tokenizer.fetch(); // nth param
        }
        expect(TT_ARROW, String(L"->"));
    }
    visitor.definitionBodyBegin(false);
    symbolTable.insert(symbolTable.begin(), symbols);
    generateFunctionCode(true);
    symbolTable.erase(symbolTable.begin());
    visitor.definitionBodyEnd(false);
    delete symbols;
    visitor.definitionEnd(INLINE_DEFINITION);
}

void Parser::generateFunctionCode(bool expectBracet) {
    if (expectBracet) {
        while (tokenizer.getCurrent().type != TT_R_BRACKET && tokenizer.getCurrent().type != TT_EOF) {
            block();
            if (tokenizer.getCurrent().type != TT_R_BRACKET) {
                visitor.addError(tokenizer.getCurrent().line, tokenizer.getCurrent().pos, String(L"Missing Semicolon!"));
            }
        }
        expect(TT_R_BRACKET, String(L"]"));
    } else {
        statement();
    }
}


void Parser::relExp() {
    visitor.relExpBegin();
    termExp();
    while(true) {
        Atom opCode = NIL;
        if (tokenizer.getCurrent().type == TT_EQ) {
            opCode = SYMBOL_OP_EQ;
        } else if (tokenizer.getCurrent().type == TT_NE) {
            opCode = SYMBOL_OP_NE;
        } else if (tokenizer.getCurrent().type == TT_LT) {
            opCode = SYMBOL_OP_LT;
        } else if (tokenizer.getCurrent().type == TT_LTEQ) {
            opCode = SYMBOL_OP_LTQ;
        } else if (tokenizer.getCurrent().type == TT_GT) {
            opCode = SYMBOL_OP_GT;
        } else if (tokenizer.getCurrent().type == TT_GTEQ) {
            opCode = SYMBOL_OP_GTQ;
        }
        if (opCode == NIL) {
            visitor.relExpEnd();
            return;
        }
        visitor.relExpOp(tokenizer.getCurrent().tokenString, opCode);
        tokenizer.fetch(); // Read over operator
        termExp();
        visitor.relExpContinue(opCode);
    }    
}

void Parser::basicExp() {
    visitor.binaryExpBegin();
    logExp();
    while(true) {
        if (tokenizer.getCurrent().type == TT_AND) {
            visitor.basicExpOp(tokenizer.getCurrent().tokenString, SYMBOL_OP_AND);
            tokenizer.fetch();
            logExp();
            visitor.basicExpContinue(SYMBOL_OP_AND);
        } else if (tokenizer.getCurrent().type == TT_OR) {
            visitor.basicExpOp(tokenizer.getCurrent().tokenString, SYMBOL_OP_OR);
            tokenizer.fetch();
            logExp();
            visitor.basicExpContinue(SYMBOL_OP_OR);
        } else {
            visitor.basicExpEnd();
            return;
        }
    }
}

void Parser::logExp() {
    visitor.logExpBegin();
    relExp();
    while(true) {
        if (tokenizer.getCurrent().type == TT_PLUS) {
             visitor.basicExpOp(tokenizer.getCurrent().tokenString, SYMBOL_OP_AND);
            tokenizer.fetch();
            relExp();
            addCode(SYMBOL_OP_ADD);
        } else if (tokenizer.getCurrent().type == TT_MINUS) {
            fetch();
            relExp();
            addCode(SYMBOL_OP_SUB);
        } else if (tokenizer.getCurrent().type == TT_NUMBER && tokenizer.getCurrent().tokenInteger < 0) {
            addCode(SYMBOL_OP_LDC);
            addCode(makeNumber(tokenizer.getCurrent().tokenInteger));
            addCode(SYMBOL_OP_ADD);
            fetch();
        } else {
             visitor.logExpEnd();
            return;
        }
    }
}

void Parser::termExp() {
    factorExp();
    while(true) {
        if (tokenizer.getCurrent().type == TT_MUL) {
            fetch();
            factorExp();
            addCode(SYMBOL_OP_MUL);
        } else if (tokenizer.getCurrent().type == TT_DIV) {
            fetch();
            factorExp();
            addCode(SYMBOL_OP_DIV);
        } else if (tokenizer.getCurrent().type == TT_MOD) {
            fetch();
            factorExp();
            addCode(SYMBOL_OP_DIV); // TODO
        } else {
            return;
        }
    }
}

void Parser::factorExp() {
    if (tokenizer.getCurrent().type == TT_L_BRACE) {
        fetch();
        expression();
        expect(TT_R_BRACE, String(L")"));
    } else if (tokenizer.getCurrent().type == TT_NOT) {
        fetch();
        factorExp();
        addCode(SYMBOL_OP_NOT);
    } else {
        if (tokenizer.getCurrent().type == TT_SYMBOL || tokenizer.getCurrent().type == TT_STRING || tokenizer.getCurrent().type == TT_NUMBER) {
            literal();
        } else if (tokenizer.getCurrent().type == TT_LIST_START) {
            inlineList();
        } else if (tokenizer.getCurrent().type == TT_NAME) {
            if (lookahead.type == TT_L_BRACE) {
                call();
            } else if(tokenizer.getCurrent().tokenString[tokenizer.getCurrent().tokenString.length() - 1] == ':'
                      && lookahead.type != TT_R_BRACE
                      && lookahead.type != TT_R_BRACKET
                      && lookahead.type != TT_KOMMA
                      && lookahead.type != TT_EOF
                      && lookahead.type != TT_SEMICOLON) {
                call();
            } else {
                variable();
            }
        } else {
            addError(tokenizer.getCurrent().line, tokenizer.getCurrent().pos, String(L"Unexpected token: "+tokenizer.getCurrent().tokenString));
        }
    }
}

void Parser::inlineList() {
    fetch(); // #(
    addCode(SYMBOL_OP_NIL);
    while(tokenizer.getCurrent().type != TT_R_BRACE) {
        expression();
        addCode(SYMBOL_OP_CHAIN);
        if (tokenizer.getCurrent().type == TT_KOMMA) {
            fetch();
        }
    }
    addCode(SYMBOL_OP_CHAIN_END);
    expect(TT_R_BRACE, String(L")"));
}

Atom Compiler::compileLiteral() {
    Atom result;
    if (tokenizer.getCurrent().type == TT_SYMBOL) {
         result = engine->storage.makeSymbol(tokenizer.getCurrent().tokenString);
    } else if (tokenizer.getCurrent().type == TT_STRING) {
        result = engine->storage.makeString(tokenizer.getCurrent().tokenString);
    } else if (tokenizer.getCurrent().type == TT_NUMBER) {
        result = makeNumber(tokenizer.getCurrent().tokenInteger);
    } else {
        addError(line, pos, String(L"Unexpacted token! Expected a literal"));
    }
    fetch();
    return result;
}

void Parser::literal() {

    addCode(SYMBOL_OP_LDC);
    addCode(compileLiteral());
}

void Parser::variable() {
    String name = tokenizer.getCurrent().tokenString;
    fetch();
    load(name);
}

void Compiler::load(String variable) {
    std::pair<int, int> pair = findSymbol(variable);
    if (pair.first > 0) {
        addCode(SYMBOL_OP_LD);
        addCode(engine->storage.makeCons(makeNumber(pair.first), makeNumber(pair.second)));
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

void Parser::call() {

    if (tokenizer.getCurrent().tokenString[tokenizer.getCurrent().tokenString.length()-1] == ':') {
        colonCall();
    } else {
        standardCall();
    }
}

void Parser::colonCall() {
    String name;
    addCode(SYMBOL_OP_NIL);
    while(tokenizer.getCurrent().type == TT_NAME &&
          tokenizer.getCurrent().tokenString[tokenizer.getCurrent().tokenString.length()-1] == ':') {
        name += tokenizer.getCurrent().tokenString;
        fetch();
        expression();
        addCode(SYMBOL_OP_CHAIN);
    }
    addCode(SYMBOL_OP_CHAIN_END);
    load(name);
    addCode(SYMBOL_OP_AP);
}

void Parser::standardCall() {
    String name = tokenizer.getCurrent().tokenString;
    fetch(); // name
    fetch(); // (
    if (tokenizer.getCurrent().type != TT_R_BRACE) {
        addCode(SYMBOL_OP_NIL);
        while(tokenizer.getCurrent().type != TT_R_BRACE) {
            expression();
            addCode(SYMBOL_OP_CHAIN);
            if (tokenizer.getCurrent().type == TT_KOMMA) {
                fetch();
            }
        }
        addCode(SYMBOL_OP_CHAIN_END);
        expect(TT_R_BRACE, String(L")"));
        load(name);
        addCode(SYMBOL_OP_AP);
    } else {
        expect(TT_R_BRACE, String(L")"));
        load(name);
        addCode(SYMBOL_OP_AP0);
    }
}
