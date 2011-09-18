#include "parser.h"

Parser::Parser(String fileName, Tokenizer& tokenizer, AST& ast)
    : tokenizer(tokenizer), ast(ast)
{
    this->fileName = fileName;
}

void Parser::parse() {
    tokenizer.fetch();
    Node* root = ast.createChild();
    ast.makeRoot(root);
    while(tokenizer.getCurrent().type != TT_EOF) {
        block(root);
        if (tokenizer.getCurrent().type != TT_EOF) {
            addError(tokenizer.getCurrent().line,
                     tokenizer.getCurrent().pos,
                     String(L"Missing Semicolon!"));
        }
    }

}

void Parser::addError(int line, int pos, String errorMsg) {
    CompilationError e;
    e.error = errorMsg;
    e.line = line;
    e.pos = pos;
    errors.push_back(e);
}


void Parser::expect(InputTokenType tt, String rep) {
    if (tokenizer.getCurrent().type != tt) {
        addError(tokenizer.getCurrent().line,
                 tokenizer.getCurrent().pos,
                 String(L"Unexpected token: ") +
                 tokenizer.getString(tokenizer.getCurrent()).toStdWString() +
                 String(L". Expected: ")+rep
                 );
    } else {
        tokenizer.fetch();
    }
}

void Parser::block(Node* block) {
    statement(block);
    while(tokenizer.getCurrent().type == TT_SEMICOLON
          && tokenizer.getCurrent().type != TT_R_BRACE
          && tokenizer.getCurrent().type != TT_R_BRACKET
          && tokenizer.getCurrent().type != TT_EOF) {
        tokenizer.fetch();
        statement(block);
    }
    if (tokenizer.getCurrent().type == TT_SEMICOLON) {
        tokenizer.fetch();
    }
}

void Parser::statement(Node* block) {
    Node* stmt;
    if (tokenizer.getCurrent().type == TT_NAME) {
        if (tokenizer.getLookahead().type == TT_ASSIGNMENT) {
            stmt = localAssignment();
            return;
        } else if (tokenizer.getLookahead().type == TT_GLOBAL_ASSIGNMENT) {
            stmt = globalAssignment();
            return;
        }
    }
    stmt = expression();
    block->addChild("statements", stmt);
}

Node* Parser::localAssignment() {
    if (symbolTable.size() == 0) {        
        return globalAssignment();
    }
    String name = tokenizer.getString(tokenizer.getCurrent()).toStdWString();
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
    Node* value = expression();
    Node* result = ast.createChild();
    result->setCharAttribute("type","Assignment");
    result->setStringAttribute("name", name);
    result->setIntAttribute("majorIndex", 1);
    result->setIntAttribute("minorIndex", minorIndex);
    result->setChild("value", value);
    return result;
}

Node* Parser::globalAssignment() {
    String name = tokenizer.getString(tokenizer.getCurrent()).toStdWString();
    tokenizer.fetch(); // name
    tokenizer.fetch(); // ::=
    Node* value = expression();
    std::pair<int, int> pair = findSymbol(name);
    if (pair.first > 0) {
        Node* result = ast.createChild();
        result->setCharAttribute("type","Assignment");
        result->setStringAttribute("name", name);
        result->setIntAttribute("majorIndex", pair.first);
        result->setIntAttribute("minorIndex", pair.second);
        result->setChild("value", value);
        return result;
    } else {
        Node* result = ast.createChild();
        result->setCharAttribute("type","GlobalAssignment");
        result->setStringAttribute("name", name);
        result->setChild("value", value);
        return result;
    }
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

Node* Parser::expression() {
    if (tokenizer.getCurrent().type == TT_L_BRACE
            && tokenizer.getLookahead().type == TT_NAME
            && tokenizer.getLookahead2().type == TT_KOMMA)
    {
       return definition();
    } else if (tokenizer.getCurrent().type == TT_NAME
               && tokenizer.getLookahead().type == TT_ARROW)
    {
        return shortDefinition();
    } else if (tokenizer.getCurrent().type == TT_L_BRACKET)
    {
        return inlineDefinition();
    } else {
       return basicExp();
    }
}

Node* Parser::definition() {
    Node* result = ast.createChild();
    result->setCharAttribute("type","Definition");
    fetch(); // (
    std::vector<String>* symbols = new std::vector<String>();
    Node* param = ast.createChild();
    param->setStringAttribute("name", tokenizer.getCurrent().tokenString);
    result->addChild("params", param);
    symbols->push_back(tokenizer.getCurrent().tokenString);
    fetch(); // 1st param
    while(tokenizer.getCurrent().type == TT_KOMMA) {
        fetch(); // ,
        param = ast.createChild();
        param->setStringAttribute("name", tokenizer.getCurrent().tokenString);
        result->addChild("params", param);
        symbols->push_back(tokenizer.getCurrent().tokenString);
        fetch(); // nth param
    }
    expect(TT_R_BRACE, String(L")"));
    expect(TT_ARROW, String(L"->"));
    symbolTable.insert(symbolTable.begin(), symbols);
    if (tokenizer.getCurrent().type == TT_L_CURLY) {
        generateGuardedFunctionCode(result);
    } else {
        bool brackets = false;
        if (tokenizer.getCurrent().type == TT_L_BRACKET) {
            fetch();
            brackets = true;
        }
        generateFunctionCode(result, brackets);
    }
    symbolTable.erase(symbolTable.begin());
    delete symbols;

    return result;
}

void Parser::generateGuardedFunctionCode(Node* f) {
    Node* result = ast.createChild();
    result->setCharAttribute("type","GuardedFunction");
    expect(TT_L_CURLY, String(L"{"));
    do {
        expect(TT_L_BRACKET, String(L"["));
        if (current.type != TT_COLON) {
            result->setChild("gurad",basicExp());
        }
        expect(TT_COLON, String(L":"));
        generateFunctionCode(result, true);
    } while(current.type == TT_L_BRACKET);
    expect(TT_R_CURLY, String(L"}"));
    f->addChild("guards", result);
}

Node* Parser::shortDefinition() {
    Node* result = ast.createChild();
    result->setCharAttribute("type","Definition");

    std::vector<String>* symbols = new std::vector<String>();
    Node* param = ast.createChild();
    param->setStringAttribute("name", tokenizer.getCurrent().tokenString);
    result->addChild("params", param);
    symbols->push_back(tokenizer.getCurrent().tokenString);
    fetch(); // param name...
    expect(TT_ARROW, String(L"->"));
    symbolTable.insert(symbolTable.begin(), symbols);
    if (tokenizer.getCurrent().type == TT_L_CURLY) {
        generateGuardedFunctionCode(result);
    } else {
        bool brackets = false;
        if (tokenizer.getCurrent().type == TT_L_BRACKET) {
            fetch();
            brackets = true;
        }
        generateFunctionCode(result, brackets);
    }
    symbolTable.erase(symbolTable.begin());
    delete symbols;

    return result;
}

Node* Parser::inlineDefinition() {
    Node* result = ast.createChild();
    result->setCharAttribute("type","Definition");

    fetch(); // [
    std::vector<String>* symbols = new std::vector<String>();
    if (tokenizer.getCurrent().type == TT_NAME && (tokenizer.getLookahead().type == TT_KOMMA || tokenizer.getLookahead2().type == TT_ARROW)) {
        Node* param = ast.createChild();
        param->setStringAttribute("name", tokenizer.getCurrent().tokenString);
        result->addChild("params", param);
        symbols->push_back(tokenizer.getCurrent().tokenString);
        fetch(); // 1st param
        while(tokenizer.getCurrent().type == TT_KOMMA) {
            fetch(); // ,
            param = ast.createChild();
            param->setStringAttribute("name", tokenizer.getCurrent().tokenString);
            result->addChild("params", param);
            symbols->push_back(tokenizer.getCurrent().tokenString);
            fetch(); // nth param
        }
        expect(TT_ARROW, String(L"->"));
    }
    symbolTable.insert(symbolTable.begin(), symbols);
    generateFunctionCode(result, true);
    symbolTable.erase(symbolTable.begin());
    delete symbols;

    return result;
}

void Parser::generateFunctionCode(Node* block, bool expectBracet) {
    if (expectBracet) {
        while (tokenizer.getCurrent().type != TT_R_BRACKET
               && tokenizer.getCurrent().type != TT_EOF) {
            block(block);
            if (tokenizer.getCurrent().type != TT_R_BRACKET) {
                addError(tokenizer.getCurrent().line,
                         tokenizer.getCurrent().pos,
                         String(L"Missing Semicolon!"));
            }
        }
        expect(TT_R_BRACKET, String(L"]"));
    } else {
        statement(block);
    }
}


Node* Parser::relExp() {
    Node* result = ast.createChild();
    while(true) {
        Node* tmp = result;
        Node* result = ast.createChild();
        result->setChild("left", termExp());
        result->setCharAttribute("type","Operation");
        if (tokenizer.getCurrent().type == TT_EQ) {
            result->setCharAttribute("operator","==");
        } else if (tokenizer.getCurrent().type == TT_NE) {
            result->setCharAttribute("operator","==");
        } else if (tokenizer.getCurrent().type == TT_LT) {
            result->setCharAttribute("operator","<");
        } else if (tokenizer.getCurrent().type == TT_LTEQ) {
            result->setCharAttribute("operator","<=");
        } else if (tokenizer.getCurrent().type == TT_GT) {
            result->setCharAttribute("operator",">");
        } else if (tokenizer.getCurrent().type == TT_GTEQ) {
            result->setCharAttribute("operator",">=");
        } else {
            return;
        }
        fetch(); // Read over operator
        /*
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
        */
        result->setChild("right", termExp());
        // Remember second argument in case we have a conjunction like
        // 1 < x < 10...
        //lastSubexpression = engine->storage.getCons(lastSubexpression)->cdr;
        //addCode(opCode);
        //if (conjunction) {
        //    addCode(SYMBOL_OP_AND);
        //}
    }
}

Node* parser::basicExp() {
    Node* result = ast.createChild();
    result->setChild("left", logExp());
    result->setCharAttribute("type","Operation");
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
            addCode(engine->storage.makeNumber(current.tokenInteger));
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
    while(current.type != TT_R_BRACE && current.type != TT_EOF) {
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
    Atom result = NIL;
    if (current.type == TT_SYMBOL) {
         result = engine->storage.makeSymbol(current.tokenString);
    } else if (current.type == TT_STRING) {
        result = engine->storage.makeString(current.tokenString);
    } else if (current.type == TT_NUMBER) {
        result = engine->storage.makeNumber(current.tokenInteger);
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
    fetch(); // name
    fetch(); // (
    if (current.type != TT_R_BRACE) {
        addCode(SYMBOL_OP_NIL);
        while(current.type != TT_R_BRACE && current.type != TT_EOF) {
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
    } else {
        expect(TT_R_BRACE, String(L")"));
        load(name);
        addCode(SYMBOL_OP_AP0);
    }
}

