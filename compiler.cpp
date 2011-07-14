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

void Compiler::addError(int line, int pos, String errorMsg) {
    CompilationError e;
    e.error = errorMsg;
    e.line = line;
    e.pos = pos;
    e.severe = true;
    errors.push_back(e);
}

/**
  Fetches the next token from the given stream. This method
  also updates the current position in the file and skips
  comments and whitespaces...

  As one can see in the beginning, it uses two gotos while
  parsing comments. This can propbably refactored with an
  extra method+loop.

  After that, the next chracter is read and it is decieded
  which token is to be generated.
  */
Token Compiler::fetchToken()  {
begin: // <---------------------------------------------+
    while(std::isspace(ch) && !input.eof()) {        // |
        nextChar();                                  // |
    }                                                // |
    if (ch == '/') {                                 // |
        nextChar();                                  // |
        if (ch == '*') {                             // |
            // We have a commend, read over it...       |
            nextChar();                              // |
skip: // <-----------------------------------------+    |
            while(ch != '*' && !input.eof()) {  // |    |
                nextChar();                     // |    |
            }                                   // |    |
            if (!input.eof()) {                 // |    |
                nextChar();                     // |    |
                if (ch != '/') {                // |    |
                    // There was a * in the        |    |
                    // comment. Ignore and repeat. |    |
                    goto skip; // -----------------+    |
                }                                    // |
                // We the end of the comment.           |
                // Skip / and restart "fetchToken".     |
                nextChar();                          // |
            }                                        // |
            goto begin; //------------------------------+
        } else {
            // We have a division operator...
            Token result;
            result.pos = pos - 1;
            result.line = line;
            result.tokenString = String(L"/");
            result.type = TT_DIV;
            return result;
        }
    }
    Token result;
    result.pos = pos - 1;
    result.line = line;
    result.type = TT_EMPTY;
    if (input.eof()) {
        result.type = TT_EOF;
    } else if (std::isalpha(ch)) {
        result.tokenString = String();
        result.tokenString += ch;
        result.type = TT_NAME;
        nextChar();
        while((std::isalpha(ch)
               || std::isdigit(ch)
               || ch == ':'
               || ch == '_')
              && !input.eof())
        {
            result.tokenString += ch;
            nextChar();
        }
    } else if (ch == ';') {
        result.tokenString = String(L";");
        nextChar();
        result.type = TT_SEMICOLON;
    } else if (ch == '-') {
        nextChar();
        if (ch == '>') {
            nextChar();
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
        nextChar();
        if (ch == '=') {
            nextChar();
            result.tokenString = String(L":=");
            result.type = TT_ASSIGNMENT;
        } else if (ch == ':') {
            nextChar();
            if (ch == '=') {
               nextChar();
               result.tokenString = String(L"::=");
               result.type = TT_GLOBAL_ASSIGNMENT;
            } else {
                addError(line, pos-1, L"Unexpected colon. (Names must not start with a colon)");
            }
        } else if (std::isspace(ch)){
            result.tokenString = String(L":");
            result.type = TT_COLON;
        } else {
            addError(line, pos-1, L"Unexpected colon. (Names must not start with a colon)");
        }
    } else if (ch == '(') {
        result.tokenString = String(L"(");
        nextChar();
        result.type = TT_L_BRACE;
    } else if (ch == ')') {
        result.tokenString = String(L")");
        nextChar();
        result.type = TT_R_BRACE;
    } else if (ch == '{') {
        result.tokenString = String(L"{");
        nextChar();
        result.type = TT_L_CURLY;
    } else if (ch == '}') {
        result.tokenString = String(L"}");
        nextChar();
        result.type = TT_R_CURLY;
    } else if (ch == '[') {
        result.tokenString = String(L"[");
        nextChar();
        result.type = TT_L_BRACKET;
    } else if (ch == ']') {
        result.tokenString = String(L"]");
        nextChar();
        result.type = TT_R_BRACKET;
    } else if (ch == ',') {
        result.tokenString = String(L",");
        nextChar();
        result.type = TT_KOMMA;
    } else if (ch == '.') {
        result.tokenString = String(L".");
        nextChar();
        result.type = TT_DOT;
    } else if (ch == '=') {
        result.tokenString = String(L"=");
        nextChar();
        result.type = TT_EQ;
    } else if (ch == '+') {
        result.tokenString = String(L"+");
        nextChar();
        result.type = TT_PLUS;
    } else if (ch == '&') {
        result.tokenString = String(L"&");
        nextChar();
        result.type = TT_AND;
    } else if (ch == '|') {
        result.tokenString = String(L"&");
        nextChar();
        result.type = TT_OR;
    } else if (ch == '%') {
        result.tokenString = String(L"%");
        nextChar();
        result.type = TT_MOD;
    } else if (ch == '*') {
        result.tokenString = String(L"*");
        nextChar();
        result.type = TT_MUL;
    } else if (ch == '!') {
        nextChar();
        if (ch == '=') {
            nextChar();
            result.tokenString = String(L"!=");
            result.type = TT_NE;
        } else {
            result.tokenString = String(L"!");
            result.type = TT_NOT;
        }
    } else if (ch == '<') {
        nextChar();
        if (ch == '<') {
            nextChar();
            result.tokenString = String(L"<<");
            result.type = TT_ASM_BEGIN;
        } else if (ch == '='){
            nextChar();
            result.tokenString = String(L"<=");
            result.type = TT_LTEQ;
        } else {
            result.tokenString = String(L"<");
            result.type = TT_LT;
        }
    } else if (ch == '>') {
        nextChar();
        if (ch == '>') {
            nextChar();
            result.tokenString = String(L">>");
            result.type = TT_ASM_END;
        } else if (ch == '='){
            nextChar();
            result.tokenString = String(L">=");
            result.type = TT_GTEQ;
        } else {
            result.tokenString = String(L">");
            result.type = TT_GT;
        }
    } else if (ch == '#') {
        nextChar();
        if (ch == '(') {
            nextChar();
            result.tokenString = String(L"#(");
            result.type = TT_LIST_START;
        } else {
            result.tokenString = String();
            while(std::isalnum(ch) && !input.eof()) {
                result.tokenString += ch;
                nextChar();
            }
            result.type = TT_SYMBOL;
        }
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
        nextChar();
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
        addError(result.line, result.pos, L"Unexpected end of input");
    }
    return result;
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
        addError(current.line, current.pos, String(L"Unexpected token: ")+current.tokenString+String(L". Expected: ")+rep);
    } else {
        fetch();
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
    fetch();
    addCode(SYMBOL_OP_FILE);
    addCode(file);
    while(current.type != TT_EOF) {
        block();
        if (current.type != TT_EOF) {
            addError(current.line, current.pos, String(L"Missing Semicolon!"));
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
    while(current.type == TT_SEMICOLON
          && lookahead.type != TT_R_BRACE
          && lookahead.type != TT_R_BRACKET
          && lookahead.type != TT_EOF) {
        fetch();
        statement();
    }
    if (current.type == TT_SEMICOLON) {
        fetch();
    }
}

void Compiler::statement() {
    addCode(SYMBOL_OP_LINE);
    addCode(makeNumber(line));
    if (current.type == TT_NAME) {
        if (lookahead.type == TT_ASSIGNMENT) {
            localAssignment();
            return;
        } else if (lookahead.type == TT_GLOBAL_ASSIGNMENT) {
            globalAssignment();
            return;
        }
    }
    expression();
}

void Compiler::localAssignment() {
    if (symbolTable.size() == 0) {
        globalAssignment();
        return;
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

void Compiler::globalAssignment() {
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
        basicExp();
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
        while (current.type != TT_R_BRACKET && current.type != TT_EOF) {
            block();
            if (current.type != TT_R_BRACKET) {
                addError(current.line, current.pos, String(L"Missing Semicolon!"));
            }
        }
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
    termExp();
    Atom lastSubexpression = NIL;
    while(true) {
        Atom opCode = NIL;
        if (current.type == TT_EQ) {
            opCode = SYMBOL_OP_EQ;
        } else if (current.type == TT_NE) {
            opCode = SYMBOL_OP_NE;
        } else if (current.type == TT_LT) {
            opCode = SYMBOL_OP_LT;
        } else if (current.type == TT_LTEQ) {
            opCode = SYMBOL_OP_LTQ;
        } else if (current.type == TT_GT) {
            opCode = SYMBOL_OP_GT;
        } else if (current.type == TT_GTEQ) {
            opCode = SYMBOL_OP_GTQ;
        }
        if (opCode == NIL) {
            return;
        }
        fetch(); // Read over operator
        bool conjunction = lastSubexpression != NIL;
        if (conjunction) {
            // if we're in a conjunction, like 1 < x < 10, copy last
            // argument (x in this case) so we build an expression like
            // 1 < x & x < 10
            Atom code = lastSubexpression;
            Atom stop = tail;
            while(isCons(code) && code != stop) {
                std::wcout << engine->toString(code) << std::endl;
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
        if (current.type == TT_AND) {
            fetch();
            logExp();
            addCode(SYMBOL_OP_AND);
        } else if (current.type == TT_OR) {
            fetch();
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
        if (current.type == TT_PLUS) {
            fetch();
            relExp();
            addCode(SYMBOL_OP_ADD);
        } else if (current.type == TT_MINUS) {
            fetch();
            relExp();
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
    } else if (current.type == TT_NOT) {
        fetch();
        factorExp();
        addCode(SYMBOL_OP_NOT);
    } else {
        if (current.type == TT_SYMBOL || current.type == TT_STRING || current.type == TT_NUMBER) {
            literal();
        } else if (current.type == TT_LIST_START) {
            inlineList();
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
            addError(current.line, current.pos, String(L"Unexpected token: "+current.tokenString));
        }
    }
}

void Compiler::inlineList() {
    fetch(); // #(
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
        addError(line, pos, String(L"Unexpacted token! Expected a literal"));
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

    if (current.tokenString[current.tokenString.length()-1] == ':') {
        colonCall();
    } else {
        standardCall();
    }
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
