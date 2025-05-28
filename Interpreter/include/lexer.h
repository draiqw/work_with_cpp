#ifndef LEXER_H
#define LEXER_H

#pragma once
#include <string>
#include <iostream>
#include <vector>

// Enumeration of all possible lexeme (token) types
enum LexType {
    LEX_NULL,
    LEX_AND, LEX_OR, LEX_NOT,
    LEX_PROGRAM,
    LEX_IF, LEX_ELSE,
    LEX_WHILE, LEX_DO, LEX_FOR,
    LEX_BREAK, LEX_CONTINUE,
    LEX_GOTO,
    LEX_READ, LEX_WRITE,
    LEX_INT, LEX_BOOL, LEX_REAL, LEX_STRING,    // type keywords
    LEX_TRUE, LEX_FALSE,                        // boolean constants
    LEX_ID,                                     // identifier (variable name)
    LEX_INT_CONST, LEX_REAL_CONST, LEX_STR_CONST, // literal constants
    LEX_SEMICOLON, LEX_COMMA, LEX_COLON, LEX_ASSIGN,
    LEX_EQ, LEX_NEQ, LEX_LESS, LEX_GREATER, LEX_LEQ, LEX_GEQ,
    LEX_PLUS, LEX_MINUS, LEX_TIMES, LEX_SLASH, LEX_PERCENT,
    LEX_PLUS_EQ, LEX_MINUS_EQ, LEX_TIMES_EQ, LEX_SLASH_EQ, LEX_PERCENT_EQ,
    LEX_LPAREN, LEX_RPAREN, LEX_LBRACE, LEX_RBRACE,
    // POLIZ-specific pseudo-tokens
    POLIZ_LABEL,    // label address in POLIZ 
    POLIZ_ADDRESS,  // variable address in POLIZ (for assignments and read)
    POLIZ_GO,       // unconditional jump
    POLIZ_FGO,      // conditional jump (jump if false)
    LEX_FIN
};

// Class representing a single lexeme (token)
class Lex {
    LexType type;
    int value;  // numeric value or index (depends on LexType)
public:
    Lex(LexType t = LEX_NULL, int v = 0): type(t), value(v) {}
    LexType getType() const { return type; }
    int getValue() const { return value; }
    friend std::ostream& operator<<(std::ostream& out, const Lex& lex);
};

// Lexical Analyzer class (Scanner)
class Lexer {
    std::string input;   // entire input text
    size_t pos;          // current position in input
    int curr_line;       // current line number (for error messages)
    char curr_char;      // current character being analyzed

public:
    Lexer(const std::string &text);
    Lex getLex();        // get the next lexeme from input

private:
    void nextChar();           // advance to the next character
    void skipWhitespace();     // skip spaces, tabs, newlines
    void skipComment();        // skip comments starting with '#'
};

// External tables for string and real constants, used by Lexer/Parser
extern std::vector<std::string> string_const_table;
extern std::vector<double> real_const_table;

#endif
