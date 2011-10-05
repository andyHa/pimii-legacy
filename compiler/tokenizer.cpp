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
    inXMLNode = false;
    inXMLValueBlock = false;

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
    InputToken result;
    result.absolutePos = absolutePos;
    result.pos = pos;
    result.line = line;

    if (parseXMLContent(result)) {
        return result;
    }

    skipWhitespace();

    result.absolutePos = absolutePos;
    result.pos = pos;
    result.line = line;

    if (!more()) {
        result.length = 0;
        result.type = TT_EOF;
        return result;
    }

    if (parseXML(result)) {
        return result;
    }

    if (ch.isLetter()) {
        return parseName(TT_NAME, false);
    } else if (ch == '#' && hasPreview() && preview().isLetter()) {
        return parseSymbol();
    } else if(ch.isDigit() ||
              (ch == '-' && hasPreview() && preview().isDigit())) {
        return parseNumber();
    } else if (ch == '\'') {
        return parseString();
    } else if (ch == '/'
               && hasPreview()
               && preview() == '/') {
        if (ignoreComments) {
            parseComment();
            return fetchToken();
        } else {
            return parseComment();
        }
    } else {
        return parseOperator();
    }
}

bool Tokenizer::parseXMLContent(InputToken& token) {
    if (inXMLNode || inXMLValueBlock) {
        return false;
    }

    if (more() &&
        !cDataStack.empty() &&
        cDataStack.back() &&
        ch != '<')
    {
        parseXMLValue(token, '<');
        return true;
    }

    return false;
}

bool Tokenizer::handleXMLValueBlockEnd(InputToken& token) {
    //We're in an interpreted XML parameter like <xml x="[3+3]" />
    //Check if we're at the end ]
    if (ch == ']' && hasPreview() && preview() == '"') {
        inXMLValueBlock = false;
        nextChar();
        nextChar();
        token.length = 2;
        token.type = TT_TAG_BLOCK_END;
        return true;
     }
    return false;
}

bool Tokenizer::parseXMLNode(InputToken& token) {
    if (ch == '/') {
        currentNodeIsClosed = true;
        nextChar();
        token.length = 1;
        token.type = TT_TAG_CLOSE;
        return true;
    } else if (ch == '>') {
        inXMLNode = false;
        nextChar();
        token.length = 1;
        token.type = TT_TAG_END;
        if (cDataStack.size() > 0 && currentNodeIsEndNode) {
            cDataStack.pop_back();
        } else if (!currentNodeIsClosed && !currentNodeIsEndNode) {
            cDataStack.push_back(ch != '[');
            if (ch == '[') {
                nextChar();
                token.length = 2;
            }
        }
        return true;
    } else if (ch == '=') {
        nextChar();
        token.length = 1;
        token.type = TT_TAG_EQ;
        return true;
    } else if (ch == '"') {
        if (hasPreview() && preview() == '[') {
            inXMLValueBlock = true;
            nextChar();
            nextChar();
            token.length = 2;
            token.type = TT_TAG_BLOCK_BEGIN;
            return true;
        } else {
            nextChar();
            parseXMLValue(token, '"');
            nextChar();
            token.length +=2;
            return true;
        }
    } else if (ch.isLetter()) {
        token = parseName(TT_TAG_NAME, true);
        return true;
    }

    return false;
}

bool Tokenizer::parseXMLNodeStart(InputToken& token) {
    if (!cDataStack.empty() &&
           !cDataStack.back() &&
           ch == ']' &&
           hasPreview() &&
           preview() == '<')
    {
        // We're in an interpreted data section and  reached the ]</...>
        nextChar();
        nextChar();
        inXMLNode = true;
        currentNodeIsClosed = (ch == '/');
        currentNodeIsEndNode = (ch == '/');
        token.length = 2;
        token.type = TT_TAG_START;
        return true;
    } else if (ch == '<' &&
               hasPreview() &&
               (preview().isLetter() || preview() == '/') &&
               !inXMLNode)
    {
        // We're at the end of an cdata section and reached the </...
       nextChar();
       inXMLNode = true;
       currentNodeIsClosed = (ch == '/');
       currentNodeIsEndNode = (ch == '/');
       token.length = 1;
       token.type = TT_TAG_START;
       return true;
    }

    return false;
}

bool Tokenizer::parseXML(InputToken& token) {
    if (inXMLValueBlock) {
        if (handleXMLValueBlockEnd(token)) {
            return true;
        }
    } else if (inXMLNode) {
        if (parseXMLNode(token)) {
            return true;
        }
    } else {
        if (parseXMLNodeStart(token)) {
            return true;
        }
    }
    return false;
}

InputToken Tokenizer::parseName(InputTokenType type, bool acceptDashes) {
    InputToken result;
    result.absolutePos = absolutePos;
    result.pos = pos;
    result.line = line;
    result.length = 1;
    result.type = type;
    nextChar();
    while((ch.isLetterOrNumber()
           || ch == ':'
           || ch == '_'
           || (ch == '-' && acceptDashes))
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
    while(ch != '\'' && more())
    {
        result.length++;
        nextChar();
    }
    result.length++;
    nextChar();
    return result;
}

void Tokenizer::parseXMLValue(InputToken& token, char stopChar) {
    token.length = 0;
    token.type = TT_TAG_VALUE;
    while(ch != stopChar && more())
    {
        token.length++;
        nextChar();
    }
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
        if (hasPreview() && preview() == '&'){
            nextChar();
            result.length = 2;
            result.type = TT_AND;
        } else {
            result.type = TT_CONCAT;
        }
    } else if (ch == '|') {
        if (hasPreview() && preview() == '|'){
            nextChar();
            result.length = 2;
            result.type = TT_OR;
        } else {
            result.type = TT_SPLIT;
        }
    } else if (ch == '%') {
        result.type = TT_MOD;
    } else if (ch == '*') {
        result.type = TT_MUL;
    } else if (ch == ':') {
        if (hasPreview() && preview() == '=') {
            nextChar();
            result.length = 2;
            result.type = TT_ASSIGNMENT;
        } else if (hasPreview() && preview() == ':') {
            if (absolutePos < input.length() - 2 &&
                    input[absolutePos+2] == '=')
            {
                nextChar();
                nextChar();
                result.length = 3;
                result.type = TT_GLOBAL_ASSIGNMENT;
            }
        } else {
            result.type = TT_COLON;
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

bool Tokenizer::isCurrent(InputTokenType type) {
    return getCurrent().type == type;
}

bool Tokenizer::isLookahead(InputTokenType type) {
    return getLookahead().type == type;
}

bool Tokenizer::isLookahead2(InputTokenType type) {
    return getLookahead2().type == type;
}

QString Tokenizer::getString(InputToken token) {
    return input.mid(token.absolutePos, token.length);
}

QString Tokenizer::getCurrentString() {
    if (current.type == TT_EOF) {
        return QString("(End of Input)");
    }
    if (current.type == TT_EMPTY) {
        return QString("No Input!");
    }
    return getString(current);
}
