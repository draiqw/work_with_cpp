#ifndef IDENT_H
#define IDENT_H

#include <string>
#include <variant>
#include <vector>
#include "lexer.h"

// Variant type to hold a variable's value (int, double, string, bool)
using Value = std::variant<int, double, std::string, bool>;

// Symbol table entry for an identifier (variable)
struct Ident {
    std::string name;
    LexType type;    // declared type of the identifier (LEX_INT, LEX_REAL, etc.)
    bool declare;    // true if declared
    bool assign;     // true if a value has been assigned
    Value value;     // current value

    Ident(): type(LEX_NULL), declare(false), assign(false), value(0) {}
    Ident(const std::string& nm, LexType tp): name(nm), type(tp), declare(true), assign(false), value(0) {}
};

// Global table of identifiers (symbol table)
extern std::vector<Ident> TID;

// Add a new identifier to the table or return index if it already exists
int putIdent(const std::string& name);

#endif
