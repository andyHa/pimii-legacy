#include "tokenizer.h"

Tokenizer::Tokenizer(const QString& source, bool ignoreComments) :
    ignoreComments(ignoreComments),
    input(source),
    ch(input[0])
{
    absolutePos = -1;
    pos = 1;
    line = 1;
    current.type = TT_EMPTY;

    //Move to first non space character...
    nextChar();
    skipWhitespace();
}

QChar Tokenizer::nextChar() {
    if (absolutePos < input.length()) {
        absolutePos++;
        pos++;
        ch = input[absolutePos];
        if (ch == '\n') {
            line++;
            pos = 1;
        }
    }
    return ch;
}

void Tokenizer::skipWhitespace() {
    while(ch.isSpace() && more()) {
        nextChar();
    }
}

bool Tokenizer::more() {
    return absolutePos < input.length();
}


bool Tokenizer::hasPreview() {
    return absolutePos < input.length() - 1;
}

QChar Tokenizer::preview()  {
    return input[absolutePos+1];
}


InputToken Tokenizer::fetchToken()  {
    skipWhitespace();

    InputToken result;
    result.absolutePos = absolutePos;
    result.pos = pos;
    result.line = line;

    if (!more()) {
        result.length = 0;
        result.type = TT_EOF;
        return result;
    } else if (ch.isLetter()) {
        return parseName();
    } else if (ch == '#' && hasPreview() && preview().isLetter()) {
        return parseSymbol();
    } else if(ch.isDigit() ||
              (ch == '-' && hasPreview() && preview().isDigit())
              )
    {
        return parseNumber();
    } else if (ch == '"') {
        return parseString();
    } else if (ch == '\'') {
        return parseComment();
    } else if (ch == '/'
               && hasPreview()
               && preview() == '*') {
        if (ignoreComments) {
            parseComment();
            return fetchToken();
        } else {
            return parseComment();
        }
    }

    return parseOperator();
}

InputToken Tokenizer::parseName() {
    InputToken result;
    result.absolutePos = absolutePos;
    result.pos = pos;
    result.line = line;
    result.length = 1;
    result.type = TT_NAME;
    nextChar();
    while((ch.isLetterOrNumber()
           || ch == ':'
           || ch == '_')
          && more())
    {
        result.length++;
        nextChar();
    }
    return result;
}

InputToken Tokenizer::parseSymbol() {
    InputToken result;
    result.absolutePos = absolutePos;
    result.pos = pos;
    result.line = line;
    result.length = 1;
    result.type = TT_SYMBOL;
    nextChar();
    while((ch.isLetterOrNumber()
           || ch == '_')
          && more())
    {
        result.length++;
        nextChar();
    }
    return result;
}

InputToken Tokenizer::parseNumber() {
    InputToken result;
    bool decimalSeparatorSeen = false;
    result.absolutePos = absolutePos;
    result.pos = pos;
    result.line = line;
    result.length = 1;
    result.type = TT_NUMBER;
    nextChar();
    while((ch.isDigit() || (!decimalSeparatorSeen && ch == '.')) && more())
    {
        if (ch == '.') {
            decimalSeparatorSeen = true;
            result.type = TT_DECIMAL;
        }
        result.length++;
        nextChar();
    }
    return result;
}

InputToken Tokenizer::parseString() {
    InputToken result;
    result.absolutePos = absolutePos;
    result.pos = pos;
    result.line = line;
    result.length = 1;
    result.type = TT_STRING;
    nextChar();
    while(ch != '"' && more())
    {
        result.length++;
        nextChar();
    }
    result.length++;
    nextChar();
    return result;
}

InputToken Tokenizer::parseComment() {
    InputToken result;
    result.absolutePos = absolutePos;
    result.pos = pos;
    result.line = line;
    result.length = 1;
    result.type = TT_COMMENT;
    while(ch != '\n' && more()) {
        result.length++;
        nextChar();
    }
    return result;
}

InputToken Tokenizer::parseOperator() {
    InputToken result;
    result.absolutePos = absolutePos;
    result.pos = pos;
    result.line = line;
    result.length = 1;
    result.type = TT_UNKNOWN;

    if (ch == '(') {
        result.type = TT_L_BRACE;
    } else if (ch == ')') {
        result.type = TT_R_BRACE;
    } else if (ch == '{') {
        result.type = TT_L_CURLY;
    } else if (ch == '}') {
        result.type = TT_R_CURLY;
    } else if (ch == '[') {
        result.type = TT_L_BRACKET;
    } else if (ch == ']') {
        result.type = TT_R_BRACKET;
    } else if (ch == ':') {
        result.type = TT_COLON;
    } else if (ch == ';') {
        result.type = TT_SEMICOLON;
    } else if (ch == ',') {
        result.type = TT_KOMMA;
    } else if (ch == '.') {
        result.type = TT_DOT;
    } else if (ch == '=') {
        result.type = TT_EQ;
    } else if (ch == '+') {
        result.type = TT_PLUS;
    } else if (ch == '&') {
        result.type = TT_AND;
    } else if (ch == '|') {
        result.type = TT_OR;
    } else if (ch == '%') {
        result.type = TT_MOD;
    } else if (ch == '*') {
        result.type = TT_MUL;
    } else if (ch == ':') {
        if (hasPreview() && preview() == '='){
            nextChar();
            result.length = 2;
            result.type = TT_ASSIGNMENT;
        } else if (hasPreview() && preview() == ':') {
            if (absolutePos < input.length() - 2 &&
                    input[absolutePos+2] == '='
                ) {
                nextChar();
                nextChar();
                result.length = 3;
                result.type = TT_GLOBAL_ASSIGNMENT;
            }
        }
    } else if (ch == '#') {
        if (hasPreview() && preview() == '('){
            nextChar();
            result.length = 2;
            result.type = TT_LIST_START;
        }
    } else if (ch == '-') {
        if (hasPreview() && preview() == '>'){
            nextChar();
            result.length = 2;
            result.type = TT_ARROW;
        } else {
            result.type = TT_MINUS;
        }
    } else if (ch == '!') {
        if (hasPreview() && preview() == '='){
            nextChar();
            result.length = 2;
            result.type = TT_NE;
        } else {
            result.type = TT_NOT;
        }
    } else if (ch == '<') {
        if (hasPreview() && preview() == '='){
            nextChar();
            result.length = 2;
            result.type = TT_LTEQ;
        } else {
            result.type = TT_LT;
        }
    } else if (ch == '>') {
        if (hasPreview() && preview() == '='){
            nextChar();
            result.length = 2;
            result.type = TT_GTEQ;
        } else {
            result.type = TT_GT;
        }
    }

    nextChar();
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

QString Tokenizer::getString(InputToken token) {
    return input.mid(token.absolutePos, token.length);
}
