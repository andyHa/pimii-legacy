#include "tokenizer.h"

Tokenizer::Tokenizer(std::wistream& inputStream) : input(inputStream)
{
    absolutePos = 0;
    pos = 1;
    line = 1;
    current.type = TT_EMPTY;

    //Move to first non space character...
    nextChar();
    while(std::isspace(ch) && !input.eof()) {
        nextChar();
    }
}

wchar_t Tokenizer::nextChar() {
    input.get(ch);
    absolutePos++;
    pos++;
    if (ch == '\n') {
        line++;
        pos = 1;
    }
    return ch;
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
InputToken Tokenizer::fetchToken()  {
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
            InputToken result;
            result.absolutePos = absolutePos - 1;
            result.pos = pos - 1;
            result.line = line;
            result.tokenString = String(L"/");
            result.type = TT_DIV;
            return result;
        }
    }
    InputToken result;
    result.absolutePos = absolutePos - 1;
    result.pos = pos - 1;
    result.line = line;
    result.type = TT_EOF;
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
            while(std::isdigit(ch) && !input.eof()) {
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
            }
            result.tokenString = String(L"::=");
            result.type = TT_GLOBAL_ASSIGNMENT;
        } else {
            result.tokenString = String(L":");
            result.type = TT_COLON;
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
    return result;
}

InputToken Tokenizer::fetch() {
    if (current.type == TT_EMPTY) {
        current = fetchToken();
        lookahead = fetchToken();
        lookahead2 = fetchToken();
    } else {
        current = lookahead;
        lookahead = lookahead2;
        lookahead2 = fetchToken();
    }
    return current;
}

InputToken Tokenizer::getCurrent() {
     if (current.type == TT_EMPTY) {
         fetch();
     }
    return current;
}

InputToken Tokenizer::getLookahead() {
    if (current.type == TT_EMPTY) {
        fetch();
    }
    return lookahead;
}

InputToken Tokenizer::getLookahead2() {
    if (current.type == TT_EMPTY) {
        fetch();
    }
    return lookahead2;
}
