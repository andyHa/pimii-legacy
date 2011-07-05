#include "compiler.h"

Compiler::Compiler(String fileName, std::wistream& inputStream, Engine* engine) : input(inputStream), engine(engine)
{
    file = engine->storage.makeSymbol(fileName);
    current.type = TT_EMPTY;
    pos = 1;
    line = 1;
    //Move to first non space character...
    nextChar();
    while(std::isspace(ch)) {
        nextChar();
    }
}

wchar_t Compiler::nextChar() {
    input.get(ch);
    pos++;
    if (ch == '\n') {
        line++;
        pos = 1;
    }
    return ch;
}

Token Compiler::fetchToken()  {
    while(std::isspace(ch) && !input.eof()) {
        nextChar();
    }
    Token result;
    result.pos = pos - 1;
    result.line = line;
    result.type = TT_EMPTY;
    if (input.eof()) {
        result.type = TT_EOF;
    } else if (std::isalpha(ch) || ch == '$') {
        result.tokenString = String();
        result.tokenString += ch;
        result.type = TT_NAME;
        nextChar();
        while((std::isalpha(ch) || std::isdigit(ch) || ch == ':') && !input.eof()) {
            result.tokenString += ch;
            nextChar();
        }
    } else if (ch == ';') {
        result.tokenString = String(L";");
        nextChar(); // Read over character
        result.type = TT_SEMICOLON;
    } else if (ch == '-') {
        nextChar(); // Read over character
        if (ch == '>') {
            nextChar(); // Read over character
            result.tokenString = String(L"->");
            result.type = TT_ARROW;
        } else if (std::isdigit(ch)) {
            result.tokenString = String(L"-");
            result.tokenString += ch;
            nextChar();
            while(std::isdigit(ch)) {
                result.tokenString += ch;
                nextChar();
            }
            std::wstringstream buffer;
            buffer << result.tokenString;
            buffer >> result.tokenInteger;
            result.type = TT_NUMBER;
        } else {
            result.tokenString = String(L"-");
            result.type = TT_MINUS;
        }
    } else if (ch == ':') {
        nextChar(); // Read over character
        if (ch == '=') {
            nextChar(); // Read over character
            result.tokenString = String(L":=");
            result.type = TT_ASSIGNMENT;
        } else {
            throw new ParseException(line, pos - 1, String(L"Unexpected colon. (Names must not start with a colon)"));
        }
    } else if (ch == '(') {
        result.tokenString = String(L"(");
        nextChar(); // Read over character
        result.type = TT_L_BRACE;
    } else if (ch == ')') {
        result.tokenString = String(L")");
        nextChar(); // Read over character
        result.type = TT_R_BRACE;
    } else if (ch == '[') {
        result.tokenString = String(L"[");
        nextChar(); // Read over character
        result.type = TT_L_BRACKET;
    } else if (ch == ']') {
        result.tokenString = String(L"]");
        nextChar(); // Read over character
        result.type = TT_R_BRACKET;
    } else if (ch == ',') {
        result.tokenString = String(L",");
        nextChar(); // Read over character
        result.type = TT_KOMMA;
    } else if (ch == '.') {
        result.tokenString = String(L".");
        nextChar(); // Read over character
        result.type = TT_DOT;
    } else if (ch == '=') {
        result.tokenString = String(L"=");
        nextChar(); // Read over character
        result.type = TT_EQ;
    } else if (ch == '+') {
        result.tokenString = String(L"+");
        nextChar(); // Read over character
        result.type = TT_PLUS;
    } else if (ch == '&') {
        result.tokenString = String(L"&");
        nextChar(); // Read over character
        result.type = TT_AND;
    } else if (ch == '|') {
        result.tokenString = String(L"&");
        nextChar(); // Read over character
        result.type = TT_OR;
    } else if (ch == '%') {
        result.tokenString = String(L"%");
        nextChar(); // Read over character
        result.type = TT_MOD;
    } else if (ch == '*') {
        result.tokenString = String(L"*");
        nextChar(); // Read over character
        result.type = TT_MUL;
    } else if (ch == '/') {        
        result.tokenString = String(L"/");
        nextChar(); // Read over character
        result.type = TT_DIV;
    } else if (ch == '!') {
        nextChar(); // Read over character
        if (ch == '=') {
            nextChar(); // Read over character
            result.tokenString = String(L"!=");
            result.type = TT_NE;
        } else {
            result.tokenString = String(L"!");
            result.type = TT_NOT;
        }
    } else if (ch == '<') {
        nextChar(); // Read over character
        if (ch == '<') {
            nextChar(); // Read over character
            result.tokenString = String(L"<<");
            result.type = TT_ASM_BEGIN;
        } else if (ch == '='){
            nextChar(); // Read over character
            result.tokenString = String(L"<=");
            result.type = TT_LTEQ;
        } else {
            result.tokenString = String(L"<");
            result.type = TT_LT;
        }
    } else if (ch == '>') {
        nextChar(); // Read over character
        if (ch == '>') {
            nextChar(); // Read over character
            result.tokenString = String(L">>");
            result.type = TT_ASM_END;
        } else if (ch == '='){
            nextChar(); // Read over character
            result.tokenString = String(L">=");
            result.type = TT_GTEQ;
        } else {
            result.tokenString = String(L">");
            result.type = TT_GT;
        }
    } else if (ch == '#') {
        result.tokenString = String();
        nextChar();
        while(std::isalnum(ch) && !input.eof()) {
            result.tokenString += ch;
            nextChar();
        }
        result.type = TT_SYMBOL;
    } else if (std::isdigit(ch)) {
        result.tokenString = String();
        result.tokenString += ch;
        nextChar();
        while(std::isdigit(ch) && !input.eof()) {
            result.tokenString += ch;
            nextChar();
        }
        std::wstringstream buffer;
        buffer << result.tokenString;
        buffer >> result.tokenInteger;
        result.type = TT_NUMBER;
    } else if (ch == '\'') {
        result.tokenString = String();
        input >> ch;
        while(ch != '\'' && !input.eof()) {
            if (ch == '\\') {
                nextChar();
            }
            result.tokenString += ch;
            nextChar();
        }
        nextChar(); // Read over last "
        result.type = TT_STRING;
    }
    if (result.type == TT_EMPTY) {
        throw new ParseException(result.line, result.pos, String(L"Unexpected token"));
    } else {
        return result;
    }
}

void Compiler::fetch() {
    if (current.type == TT_EMPTY) {
        current = fetchToken();
        lookahead = fetchToken();
        lookahead2 = fetchToken();
    } else {
        current = lookahead;
        lookahead = lookahead2;
        lookahead2 = fetchToken();
    }
}

void Compiler::expect(TokenType tt, String rep) {
    if (current.type != tt) {
        throw new ParseException(current.line, current.pos, String(L"Unexpected token: "+current.tokenString)+String(L". Expected: ")+rep);
    }
    fetch();
}

void Compiler::addCode(Atom atom) {
    if (isNil(code)) {
        code = engine->storage.makeCons(atom, NIL);
        tail = code;
    } else {
        tail = engine->storage.cons(engine->storage.getCons(tail), atom);
    }
}

Atom Compiler::compile() {
    code = NIL;
    fetch();
    addCode(SYMBOL_OP_FILE);
    addCode(file);
    block();
    return code;
}

void Compiler::block() {
    statement();
    while(current.type == TT_SEMICOLON) {
        fetch();
        statement();
    }
}

void Compiler::statement() {
    addCode(SYMBOL_OP_LINE);
    addCode(makeNumber(line));
    if (current.type == TT_NAME) {
        if (lookahead.type == TT_ASSIGNMENT) {
            normalAssignment();
            return;
        } else if (current.tokenString == String(L"var") && lookahead.type == TT_NAME && lookahead2.type == TT_ASSIGNMENT) {
            localAssignment();
            return;
        }
    }
    expression();
}

void Compiler::localAssignment() {
    if (symbolTable.size() == 0) {
        throw new ParseException(line,pos, String(L"Local Variable assignments are only allowed within functions!"));
    }
    fetch();
    String name = current.tokenString;
    fetch();
    fetch();
    expression();
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
    addCode(SYMBOL_OP_ST);
    addCode(engine->storage.makeCons(makeNumber(1), makeNumber(minorIndex)));
}

void Compiler::normalAssignment() {
    String name = current.tokenString;
    fetch();
    fetch();
    expression();
    std::pair<int, int> pair = findSymbol(name);
    if (pair.first > 0) {
        addCode(SYMBOL_OP_ST);
        addCode(engine->storage.makeCons(makeNumber(pair.first), makeNumber(pair.second)));
    } else {
        addCode(SYMBOL_OP_STG);
        addCode(engine->storage.findGlobal(engine->storage.makeSymbol(name)));
    }
}

std::pair<int, int> Compiler::findSymbol(String name) {
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

void Compiler::expression() {
    if (current.type == TT_L_BRACE && lookahead.type == TT_NAME && lookahead2.type == TT_KOMMA) {
        definition();
    } else if (current.type == TT_NAME && lookahead.type == TT_ARROW) {
        shortDefinition();
    } else if (current.type == TT_L_BRACKET) {
        inlineDefinition();
    } else if (current.type == TT_ASM_BEGIN) {
        includeAsm();
    } else {
        relExp();
    }
}

void Compiler::includeAsm() {
    fetch();
    while(current.type != TT_ASM_END) {
        if (current.type == TT_L_BRACE) {
            handleAsmSublist();
        } else {
            addCode(compileLiteral());
        }
    }
    expect(TT_ASM_END, String(L">>"));
}

void Compiler::handleAsmSublist() {
    fetch(); // (
    if (lookahead.type == TT_DOT) {
        Atom left = compileLiteral();
        fetch(); //.
        Atom right = compileLiteral();
        addCode(engine->storage.makeCons(left, right));
    } else {
        Atom backupCode = code;
        Atom backupTail = tail;
        code = NIL;
        tail = NIL;
        while(current.type != TT_R_BRACE) {
            if (current.type == TT_L_BRACE) {
                handleAsmSublist();
            } else {
                addCode(compileLiteral());
            }
        }
        Atom fn = code;
        code = backupCode;
        tail = backupTail;
        addCode(fn);
    }
    expect(TT_R_BRACE,String(L")"));

    //Linenumbers
    //Bessere Fehlermeldungen aus dem Compiler
    //Stacktraces in der VM
    //Exceptions in der VM
    //Inline Lists
    //BIFs
    //Tail-Call-Optimizer
}

void Compiler::definition() {
    fetch(); // (
    std::vector<String>* symbols = new std::vector<String>();
    symbols->push_back(current.tokenString);
    fetch(); // 1st param
    while(current.type == TT_KOMMA) {
        fetch(); // ,
        symbols->push_back(current.tokenString);
        fetch(); // nth param
    }
    expect(TT_R_BRACE, String(L")"));
    expect(TT_ARROW, String(L"->"));
    symbolTable.insert(symbolTable.begin(), symbols);
    bool brackets = false;
    if (current.type == TT_L_BRACKET) {
        fetch();
        brackets = true;
    }
    generateFunctionCode(brackets);
    symbolTable.erase(symbolTable.begin());
    delete symbols;
}

void Compiler::shortDefinition() {
    std::vector<String>* symbols = new std::vector<String>();
    symbols->push_back(current.tokenString);
    fetch(); // param name...
    expect(TT_ARROW, String(L"->"));
    symbolTable.insert(symbolTable.begin(), symbols);
    bool brackets = false;
    if (current.type == TT_L_BRACKET) {
        fetch();
        brackets = true;
    }
    generateFunctionCode(brackets);
    symbolTable.erase(symbolTable.begin());
    delete symbols;
}

void Compiler::inlineDefinition() {
    fetch(); // [
    std::vector<String>* symbols = new std::vector<String>();
    if (current.type == TT_NAME && (lookahead.type == TT_KOMMA || lookahead.type == TT_ARROW)) {
        symbols->push_back(current.tokenString);
        fetch(); // 1st param
        while(current.type == TT_KOMMA) {
            fetch(); // ,
            symbols->push_back(current.tokenString);
            fetch(); // nth param
        }
        expect(TT_ARROW, String(L"->"));
    }
    symbolTable.insert(symbolTable.begin(), symbols);
    generateFunctionCode(true);
    symbolTable.erase(symbolTable.begin());
    delete symbols;
}

void Compiler::generateFunctionCode(bool expectBracet) {
    addCode(SYMBOL_OP_LDF);
    Atom backupCode = code;
    Atom backupTail = tail;
    code = NIL;
    tail = NIL;
    addCode(SYMBOL_OP_FILE);
    addCode(file);
    if (expectBracet) {
        block();
        expect(TT_R_BRACKET, String(L"]"));
    } else {
        statement();
    }
    addCode(SYMBOL_OP_RTN);
    Atom fn = code;
    code = backupCode;
    tail = backupTail;
    addCode(fn);
}


void Compiler::relExp() {
    logExp();
    while(true) {
        if (current.type == TT_EQ) {
            fetch();
            relExp();
            addCode(SYMBOL_OP_EQ);
        } else if (current.type == TT_NE) {
            fetch();
            relExp();
            addCode(SYMBOL_OP_NE);
        } else if (current.type == TT_LT) {
            fetch();
            relExp();
            addCode(SYMBOL_OP_LT);
        } else if (current.type == TT_LTEQ) {
            fetch();
            relExp();
            addCode(SYMBOL_OP_LTQ);
        } else if (current.type == TT_GT) {
            fetch();
            relExp();
            addCode(SYMBOL_OP_GT);
        } else if (current.type == TT_GTEQ) {
            fetch();
            relExp();
            addCode(SYMBOL_OP_GTQ);
        } else {
            return;
        }
    }
}

void Compiler::logExp() {
    termExp();
    while(true) {
        if (current.type == TT_PLUS) {
            fetch();
            termExp();
            addCode(SYMBOL_OP_ADD);
        } else if (current.type == TT_MINUS) {
            fetch();
            termExp();
            addCode(SYMBOL_OP_SUB);
        } else if (current.type == TT_NUMBER && current.tokenInteger < 0) {
            addCode(SYMBOL_OP_LDC);
            addCode(makeNumber(current.tokenInteger));
            addCode(SYMBOL_OP_ADD);
            fetch();
        } else {
            return;
        }
    }
}

void Compiler::termExp() {
    factorExp();
    while(true) {
        if (current.type == TT_MUL) {
            fetch();
            factorExp();
            addCode(SYMBOL_OP_MUL);
        } else if (current.type == TT_DIV) {
            fetch();
            factorExp();
            addCode(SYMBOL_OP_DIV);
        } else if (current.type == TT_MOD) {
            fetch();
            factorExp();
            addCode(SYMBOL_OP_DIV); // TODO
        } else {
            return;
        }
    }
}

void Compiler::factorExp() {
    if (current.type == TT_L_BRACE) {
        fetch();
        expression();
        expect(TT_R_BRACE, String(L")"));
    } else {
        if (current.type == TT_SYMBOL || current.type == TT_STRING || current.type == TT_NUMBER) {
            literal();
        } else if (current.type == TT_NAME) {
            if (lookahead.type == TT_L_BRACE) {
                call();
            } else if(current.tokenString[current.tokenString.length() - 1] == ':'
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
            throw new ParseException(current.line, current.pos, String(L"Unexpected token: "+current.tokenString));
        }
    }
}

Atom Compiler::compileLiteral() {
    Atom result;
    if (current.type == TT_SYMBOL) {
         result = engine->storage.makeSymbol(current.tokenString);
    } else if (current.type == TT_STRING) {
        result = engine->storage.makeString(current.tokenString);
    } else if (current.type == TT_NUMBER) {
        result = makeNumber(current.tokenInteger);
    } else {
        throw new ParseException(line, pos, String(L"Unexpacted token! Expected a literal"));
    }
    fetch();
    return result;
}

void Compiler::literal() {
    addCode(SYMBOL_OP_LDC);
    addCode(compileLiteral());
}

void Compiler::variable() {
    String name = current.tokenString;
    fetch();
    load(name);
}

void Compiler::load(String variable) {
    std::pair<int, int> pair = findSymbol(variable);
    if (pair.first > 0) {
        addCode(SYMBOL_OP_LD);
        addCode(engine->storage.makeCons(makeNumber(pair.first), makeNumber(pair.second)));
    } else {
        addCode(SYMBOL_OP_LDG);
        addCode(engine->storage.findGlobal(engine->storage.makeSymbol(variable)));
    }
}

void Compiler::call() {

    if (current.tokenString[0] == '$') {
        builtinCall();
    } else if (current.tokenString[current.tokenString.length()-1] == ':') {
        colonCall();
    } else {
        standardCall();
    }
}

void Compiler::builtinCall() {
    String name = current.tokenString.substr(1);
    fetch();
    expect(TT_L_BRACE, String(L"("));
    while(current.type != TT_R_BRACE) {
        expression();
        if (current.type != TT_R_BRACE) {
            expect(TT_KOMMA, String(L","));
        }
    }
    expect(TT_R_BRACE, String(L")"));
    addCode(SYMBOL_OP_BAP);
    addCode(engine->findBuiltInFunction(engine->storage.makeSymbol(name)));
}

void Compiler::colonCall() {
    String name;
    addCode(SYMBOL_OP_NIL);
    while(current.type == TT_NAME &&
          current.tokenString[current.tokenString.length()-1] == ':') {
        name += current.tokenString;
        fetch();
        expression();
        addCode(SYMBOL_OP_CHAIN);
    }
    addCode(SYMBOL_OP_CHAIN_END);
    load(name);
    addCode(SYMBOL_OP_AP);
}

void Compiler::standardCall() {
    String name = current.tokenString;
    fetch();
    fetch();
    addCode(SYMBOL_OP_NIL);
    while(current.type != TT_R_BRACE) {
        expression();
        addCode(SYMBOL_OP_CHAIN);
        if (current.type == TT_KOMMA) {
            fetch();
        }
    }
    addCode(SYMBOL_OP_CHAIN_END);
    expect(TT_R_BRACE, String(L")"));
    load(name);
    addCode(SYMBOL_OP_AP);
}
