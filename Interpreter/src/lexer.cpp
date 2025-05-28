#include "lexer.h"
#include "ident.h"
#include <cctype>
#include <stdexcept>

// Define the constant tables declared in lexer.h
std::vector<std::string> string_const_table;
std::vector<double> real_const_table;

// Optional: output lexeme to stream for debugging
std::ostream& operator<<(std::ostream& out, const Lex& lex) {
    out << "Lex(" << lex.getType() << "," << lex.getValue() << ")";
    return out;
}

// Lexer constructor: initialize with input text
Lexer::Lexer(const std::string &text): input(text), pos(0), curr_line(1) {
    curr_char = (input.empty() ? '\0' : input[pos]);
}

// Advance to the next character in input
void Lexer::nextChar() {
    if (pos < input.size()) pos++;
    curr_char = (pos < input.size() ? input[pos] : '\0');
}

// Skip whitespace (spaces, tabs, newlines)
void Lexer::skipWhitespace() {
    while (true) {
        if (curr_char == ' ' || curr_char == '\t' || curr_char == '\r') {
            nextChar();
        } else if (curr_char == '\n') {
            curr_line++;
            nextChar();
        } else {
            break;
        }
    }
}

// Skip comments that start with '#' (ignore until end of line)
void Lexer::skipComment() {
    if (curr_char == '#') {
        while (curr_char != '\n' && curr_char != '\0') {
            nextChar();
        }
        // newline will be handled in skipWhitespace
    }
}

// Get the next lexeme (token) from input
Lex Lexer::getLex() {
    // Skip any whitespace and comments
    while (true) {
        skipWhitespace();
        if (curr_char == '#') {
            skipComment();
            continue;
        }
        break;
    }

    if (curr_char == '\0') {
        // End of input
        return Lex(LEX_FIN);
    }

    // Identifier or keyword: starts with letter or underscore
    if (std::isalpha(curr_char) || curr_char == '_') {
        std::string buf;
        while (std::isalnum(curr_char) || curr_char == '_') {
            buf.push_back(curr_char);
            nextChar();
        }
        // Check if it's a keyword or boolean literal
        if (buf == "program")    return Lex(LEX_PROGRAM);
        if (buf == "if")         return Lex(LEX_IF);
        if (buf == "else")       return Lex(LEX_ELSE);
        if (buf == "while")      return Lex(LEX_WHILE);
        if (buf == "do")         return Lex(LEX_DO);
        if (buf == "for")        return Lex(LEX_FOR);
        if (buf == "break")      return Lex(LEX_BREAK);
        if (buf == "continue")   return Lex(LEX_CONTINUE);
        if (buf == "goto")       return Lex(LEX_GOTO);
        if (buf == "read")       return Lex(LEX_READ);
        if (buf == "write")      return Lex(LEX_WRITE);
        if (buf == "int")        return Lex(LEX_INT);
        if (buf == "bool")       return Lex(LEX_BOOL);
        if (buf == "real")       return Lex(LEX_REAL);
        if (buf == "string")     return Lex(LEX_STRING);
        if (buf == "true")       return Lex(LEX_TRUE, 1);
        if (buf == "false")      return Lex(LEX_FALSE, 0);
        // Otherwise, it's an identifier (variable or label name)
        int id_index = putIdent(buf);   // add to symbol table if not present
        return Lex(LEX_ID, id_index);
    }

    // Number literal (int or real)
    if (std::isdigit(curr_char)) {
        long long intPart = 0;
        while (std::isdigit(curr_char)) {
            intPart = intPart * 10 + (curr_char - '0');
            nextChar();
        }
        if (curr_char == '.') {
            // Real number literal
            nextChar();
            // At least one digit should follow the decimal point
            if (!std::isdigit(curr_char)) {
                throw std::runtime_error("Lexical error: malformed real constant at line " + std::to_string(curr_line));
            }
            std::string numStr = std::to_string(intPart);
            numStr.push_back('.');
            while (std::isdigit(curr_char)) {
                numStr.push_back(curr_char);
                nextChar();
            }
            // Convert to double and store in constants table
            double realVal = std::stod(numStr);
            real_const_table.push_back(realVal);
            int idx = real_const_table.size() - 1;
            return Lex(LEX_REAL_CONST, idx);
        } else {
            // Integer literal
            return Lex(LEX_INT_CONST, static_cast<int>(intPart));
        }
    }

    // String literal
    if (curr_char == '"') {
        nextChar();
        std::string strVal;
        while (curr_char != '\0' && curr_char != '"') {
            if (curr_char == '\n') {
                throw std::runtime_error("Lexical error: newline in string literal at line " + std::to_string(curr_line));
            }
            strVal.push_back(curr_char);
            nextChar();
        }
        if (curr_char != '"') {
            throw std::runtime_error("Lexical error: missing closing '\"' for string literal at line " + std::to_string(curr_line));
        }
        nextChar();  // consume closing quote
        // Store string in constants table
        string_const_table.push_back(strVal);
        int idx = string_const_table.size() - 1;
        return Lex(LEX_STR_CONST, idx);
    }

    // Single-character and multi-character symbols/operators
    switch (curr_char) {
        case ';': nextChar(); return Lex(LEX_SEMICOLON);
        case ',': nextChar(); return Lex(LEX_COMMA);
        case ':': nextChar(); return Lex(LEX_COLON);
        case '{': nextChar(); return Lex(LEX_LBRACE);
        case '}': nextChar(); return Lex(LEX_RBRACE);
        case '(': nextChar(); return Lex(LEX_LPAREN);
        case ')': nextChar(); return Lex(LEX_RPAREN);
        case '+':
            nextChar();
            if (curr_char == '=') { nextChar(); return Lex(LEX_PLUS_EQ); }
            else return Lex(LEX_PLUS);
        case '-':
            nextChar();
            if (curr_char == '=') { nextChar(); return Lex(LEX_MINUS_EQ); }
            else return Lex(LEX_MINUS);
        case '*':
            nextChar();
            if (curr_char == '=') { nextChar(); return Lex(LEX_TIMES_EQ); }
            else return Lex(LEX_TIMES);
        case '/':
            nextChar();
            if (curr_char == '=') { nextChar(); return Lex(LEX_SLASH_EQ); }
            else return Lex(LEX_SLASH);
        case '%':
            nextChar();
            if (curr_char == '=') { nextChar(); return Lex(LEX_PERCENT_EQ); }
            else return Lex(LEX_PERCENT);
        case '=':
            nextChar();
            if (curr_char == '=') { nextChar(); return Lex(LEX_EQ); }
            else return Lex(LEX_ASSIGN);
        case '!':
            nextChar();
            if (curr_char == '=') { nextChar(); return Lex(LEX_NEQ); }
            else return Lex(LEX_NOT);
        case '<':
            nextChar();
            if (curr_char == '=') { nextChar(); return Lex(LEX_LEQ); }
            else return Lex(LEX_LESS);
        case '>':
            nextChar();
            if (curr_char == '=') { nextChar(); return Lex(LEX_GEQ); }
            else return Lex(LEX_GREATER);
        case '&':
            nextChar();
            if (curr_char == '&') { nextChar(); return Lex(LEX_AND); }
            break;  // error if single '&'
        case '|':
            nextChar();
            if (curr_char == '|') { nextChar(); return Lex(LEX_OR); }
            break;
    }

    // If we reach here, it's an unknown or unsupported symbol
    char errChar = curr_char;
    throw std::runtime_error(std::string("Lexical error: unrecognized symbol '") + errChar +
                             "' at line " + std::to_string(curr_line));
}
