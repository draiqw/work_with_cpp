#include "lexer.h"
#include <sstream>

// Initialize the map of keywords to TokenType
void Lexer::initKeywords() {
    keywordMap = {
        {"program", TokenType::KW_PROGRAM},
        {"int", TokenType::KW_INT}, {"real", TokenType::KW_REAL},
        {"bool", TokenType::KW_BOOL}, {"string", TokenType::KW_STRING},
        {"if", TokenType::KW_IF}, {"else", TokenType::KW_ELSE},
        {"while", TokenType::KW_WHILE}, {"do", TokenType::KW_DO},
        {"for", TokenType::KW_FOR}, {"case", TokenType::KW_CASE},
        {"default", TokenType::KW_DEFAULT}, {"break", TokenType::KW_BREAK},
        {"continue", TokenType::KW_CONTINUE}, {"goto", TokenType::KW_GOTO},
        {"read", TokenType::KW_READ}, {"write", TokenType::KW_WRITE},
        {"true", TokenType::BOOL_LITERAL}, {"false", TokenType::BOOL_LITERAL}
    };
}

// Skip whitespace and newline characters, update line numbers
void Lexer::skipWhitespace() {
    char c;
    while (input.get(c)) {
        if (c == '\n') {
            lineNum++;
        } else if (!isspace(c)) {
            // Push back non-whitespace character for further processing
            input.putback(c);
            return;
        }
    }
}

// Skip comments starting with '#' to end of line
void Lexer::skipComment() {
    char c;
    while (input.get(c)) {
        if (c == '\n') {
            lineNum++;
            break; // comment ends at newline
        }
        // consume characters until newline or EOF
    }
}

// Get the next token (skipping whitespace and comments)
Token Lexer::nextToken() {
    if (hasPeeked) { 
        // If we have a peeked token, return it and reset peek flag
        hasPeeked = false;
        return peekedToken;
    }
    skipWhitespace();
    char c;
    if (!(input.get(c))) {
        // End of file reached
        return Token(TokenType::END_OF_FILE, "", lineNum);
    }

    // Handle comments (line comments start with '#')
    if (c == '#') {
        skipComment();
        return nextToken(); // recurse after skipping comment
    }

    Token token;
    token.line = lineNum;
    if (isalpha(c) || c == '_') {
        // Identifier or keyword
        token = scanIdentifierOrKeyword(c);
    } 
    else if (isdigit(c)) {
        // Numeric literal (starts with digit)
        token = scanNumber(c);
    } 
    else if (c == '"') {
        // String literal
        token = scanStringLiteral();
    } 
    else {
        // Operators and symbols (single or multi-character)
        switch (c) {
            case '{': token = Token(TokenType::LBRACE, "{", lineNum); break;
            case '}': token = Token(TokenType::RBRACE, "}", lineNum); break;
            case '(': token = Token(TokenType::LPAREN, "(", lineNum); break;
            case ')': token = Token(TokenType::RPAREN, ")", lineNum); break;
            case ';': token = Token(TokenType::SEMICOLON, ";", lineNum); break;
            case ',': token = Token(TokenType::COMMA, ",", lineNum); break;
            case ':': token = Token(TokenType::COLON, ":", lineNum); break;
            case '+': token = Token(TokenType::OP_PLUS, "+", lineNum); break;
            case '-': token = Token(TokenType::OP_MINUS, "-", lineNum); break;
            case '*': token = Token(TokenType::OP_MUL, "*", lineNum); break;
            case '/': token = Token(TokenType::OP_DIV, "/", lineNum); break;
            case '%': token = Token(TokenType::OP_MOD, "%", lineNum); break;
            case '&': {
                // possibly '&&'
                char c2;
                if (input.get(c2) && c2 == '&') {
                    token = Token(TokenType::OP_AND, "&&", lineNum);
                } else {
                    // Single '&' not defined in this language, error
                    if (input) input.putback(c2);
                    token = Token(TokenType::ERROR, std::string(1, c), lineNum);
                    std::cerr << "Lexical error at line " << lineNum 
                              << ": unexpected character '&'\n";
                }
                break;
            }
            case '|': {
                // possibly '||'
                char c2;
                if (input.get(c2) && c2 == '|') {
                    token = Token(TokenType::OP_OR, "||", lineNum);
                } else {
                    if (input) input.putback(c2);
                    token = Token(TokenType::ERROR, std::string(1, c), lineNum);
                    std::cerr << "Lexical error at line " << lineNum 
                              << ": unexpected character '|'\n";
                }
                break;
            }
            case '=': {
                char c2;
                if (input.get(c2) && c2 == '=') {
                    token = Token(TokenType::OP_EQ, "==", lineNum);
                } else {
                    if (input) input.putback(c2);
                    token = Token(TokenType::ASSIGN, "=", lineNum);
                }
                break;
            }
            case '!': {
                char c2;
                if (input.get(c2) && c2 == '=') {
                    token = Token(TokenType::OP_NE, "!=", lineNum);
                } else {
                    if (input) input.putback(c2);
                    token = Token(TokenType::OP_NOT, "!", lineNum);
                }
                break;
            }
            case '<': {
                char c2;
                if (input.get(c2) && c2 == '=') {
                    token = Token(TokenType::OP_LE, "<=", lineNum);
                } else {
                    if (input) input.putback(c2);
                    token = Token(TokenType::OP_LT, "<", lineNum);
                }
                break;
            }
            case '>': {
                char c2;
                if (input.get(c2) && c2 == '=') {
                    token = Token(TokenType::OP_GE, ">=", lineNum);
                } else {
                    if (input) input.putback(c2);
                    token = Token(TokenType::OP_GT, ">", lineNum);
                }
                break;
            }
            default:
                // Unrecognized character
                token = Token(TokenType::ERROR, std::string(1, c), lineNum);
                std::cerr << "Lexical error at line " << lineNum 
                          << ": invalid character '" << c << "'\n";
                break;
        }
    }
    return token;
}

// Look ahead at next token without consuming it
Token Lexer::peekToken() {
    if (!hasPeeked) {
        peekedToken = nextToken();
        hasPeeked = true;
    }
    return peekedToken;
}

// Scan and form an identifier or keyword token (starting with firstChar)
Token Lexer::scanIdentifierOrKeyword(char firstChar) {
    std::string lexeme(1, firstChar);
    char c;
    // Continue reading alphanumeric or '_' characters
    while (input.get(c)) {
        if (isalnum(c) || c == '_') {
            lexeme.push_back(c);
        } else {
            input.putback(c);
            break;
        }
    }
    Token token;
    token.lexeme = lexeme;
    token.line = lineNum;
    // Check if lexeme is a keyword or boolean literal
    auto it = keywordMap.find(lexeme);
    if (it != keywordMap.end()) {
        TokenType type = it->second;
        token.type = type;
        if (type == TokenType::BOOL_LITERAL) {
            // "true" or "false" recognized
            token.boolVal = (lexeme == "true");
        }
    } else {
        // Not a keyword, so it is an identifier
        token.type = TokenType::IDENTIFIER;
    }
    return token;
}

// Scan a numeric literal (integer or real)
Token Lexer::scanNumber(char firstChar) {
    std::string numberText(1, firstChar);
    bool isReal = false;
    char c;
    while (input.get(c)) {
        if (isdigit(c)) {
            numberText.push_back(c);
        } else if (c == '.') {
            if (isReal) {
                // Second dot in number (error in number format)
                input.putback(c);
                break;
            }
            isReal = true;
            numberText.push_back(c);
        } else {
            input.putback(c);
            break;
        }
    }
    Token token;
    token.line = lineNum;
    if (isReal) {
        token.type = TokenType::REAL_LITERAL;
        try {
            token.realVal = std::stod(numberText);
        } catch (...) {
            token.type = TokenType::ERROR;
            std::cerr << "Lexical error at line " << lineNum 
                      << ": invalid real literal \"" << numberText << "\"\n";
        }
        token.lexeme = numberText;
    } else {
        token.type = TokenType::INTEGER_LITERAL;
        try {
            token.intVal = std::stoi(numberText);
        } catch (...) {
            token.type = TokenType::ERROR;
            std::cerr << "Lexical error at line " << lineNum 
                      << ": invalid integer literal \"" << numberText << "\"\n";
        }
        token.lexeme = numberText;
    }
    return token;
}

// Scan a string literal (assuming starting quote already consumed)
Token Lexer::scanStringLiteral() {
    std::string str;
    char c;
    bool closed = false;
    while (input.get(c)) {
        if (c == '"') {
            closed = true;
            break;
        } else if (c == '\n') {
            lineNum++;
            // string literals should not span lines in this simple language
            std::cerr << "Lexical error at line " << lineNum 
                      << ": newline in string literal\n";
            closed = true;
            break;
        } else {
            str.push_back(c);
        }
    }
    Token token;
    token.line = lineNum;
    token.type = TokenType::STRING_LITERAL;
    token.lexeme = str;
    if (!closed) {
        // If we ended input without closing quote
        token.type = TokenType::ERROR;
        std::cerr << "Lexical error at line " << lineNum 
                  << ": unterminated string literal\n";
    }
    return token;
}
