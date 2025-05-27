#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <iostream>
#include <cctype>
#include <unordered_map>

// Token types for the lexical analyzer
enum class TokenType {
    // End-of-file or invalid
    END_OF_FILE, ERROR,
    // Literals and identifiers
    IDENTIFIER, INTEGER_LITERAL, REAL_LITERAL, STRING_LITERAL, BOOL_LITERAL,
    // Keywords
    KW_PROGRAM, KW_INT, KW_REAL, KW_BOOL, KW_STRING,
    KW_IF, KW_ELSE, KW_WHILE, KW_DO, KW_FOR, KW_CASE, KW_DEFAULT,
    KW_BREAK, KW_CONTINUE, KW_GOTO, KW_READ, KW_WRITE,
    // Symbols and operators
    LBRACE, RBRACE, LPAREN, RPAREN, SEMICOLON, COMMA, COLON,
    ASSIGN,     // '='
    OP_EQ,      // '=='
    OP_NE,      // '!='
    OP_LT,      // '<'
    OP_LE,      // '<='
    OP_GT,      // '>'
    OP_GE,      // '>='
    OP_PLUS,    // '+'
    OP_MINUS,   // '-'
    OP_MUL,     // '*'
    OP_DIV,     // '/'
    OP_MOD,     // '%'
    OP_AND,     // '&&'
    OP_OR,      // '||'
    OP_NOT      // '!' (logical NOT)
};

// Structure to represent a token with type and value (if applicable)
struct Token {
    TokenType type;
    std::string lexeme;  // Original text of token (if needed for identifiers, strings, etc.)
    int intVal;          // For integer literals
    double realVal;      // For real literals
    bool boolVal;        // For boolean literals (true/false)
    int line;            // Source line number for error reporting

    Token(TokenType t = TokenType::ERROR, const std::string& lex = "", int ln = 0)
        : type(t), lexeme(lex), intVal(0), realVal(0.0), boolVal(false), line(ln) {}
};

// Lexical Analyzer class
class Lexer {
public:
    Lexer(std::istream& in) : input(in), lineNum(1) {
        initKeywords();
    }

    // Get the next token from input
    Token nextToken();

    // Peek at the next token without consuming it (optional, not strictly needed)
    Token peekToken();

private:
    std::istream& input;
    int lineNum;  // current line number in source
    Token peekedToken; 
    bool hasPeeked = false;

    // Keyword map: from string to TokenType
    std::unordered_map<std::string, TokenType> keywordMap;

    void initKeywords();
    void skipWhitespace();
    void skipComment();
    Token scanNumber(char firstChar);
    Token scanIdentifierOrKeyword(char firstChar);
    Token scanStringLiteral();
};

#endif // LEXER_H
