#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <unordered_map>
#include "lexer.h"
#include "ident.h"
#include "executor.h"

// Data type categories for expressions and variables
enum DataType { D_NONE, D_INT, D_REAL, D_BOOL, D_STRING };

class Parser {
    Lexer &lexer;
    Lex curr_lex;
    LexType curr_type;
    int curr_val;

    std::vector<Lex> poliz;               // output POLIZ code
    std::vector<Lex>* cur_poliz;          // pointer to current output vector (for handling sub-sequences)
    
    // Label handling: mapping from label name to its address in poliz, and unresolved jumps
    std::unordered_map<std::string, int> label_map;
    std::unordered_map<std::string, std::vector<int>> label_fixups;
    
    // Loop context stack (for break/continue handling)
    struct LoopCtx {
        int start_index;                 // index of loop start in poliz (for continue)
        int continue_target;             // poliz index where a "continue" should jump (top of loop or loop's iteration code)
        std::vector<int> break_positions;
        std::vector<int> continue_positions;
    };
    std::vector<LoopCtx> loop_stack;

public:
    Parser(Lexer &lex): lexer(lex), cur_poliz(&poliz) {}
    void analyze();                      // perform full parsing and POLIZ generation
    const std::vector<Lex>& getPoliz() const { return poliz; }

private:
    // Utility: read next lexeme into curr_lex, curr_type, curr_val
    void gl() {
        curr_lex = lexer.getLex();
        curr_type = curr_lex.getType();
        curr_val = curr_lex.getValue();
    }
    // Utility: emit a lexeme into current output (poliz)
    void emit(const Lex &lex) {
        cur_poliz->push_back(lex);
    }
    // Error reporting utility
    [[noreturn]] void parseError(const std::string &msg) {
        throw std::runtime_error("Syntax error: " + msg);
    }

    // Grammar parsing functions (recursive descent)
    void parseProgram();
    void parseBlock();
    void parseDeclaration();
    void parseStatement();
    DataType parseExpression();
    DataType parseOr();
    DataType parseAnd();
    DataType parseEquality();
    DataType parseRelational();
    DataType parseAdditive();
    DataType parseTerm();
    DataType parseFactor();
};

#endif
