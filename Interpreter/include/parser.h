#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <vector>
#include <string>
#include <unordered_map>

// Type enumeration for semantic analysis
enum class ValueType { INT, REAL, BOOL, STRING, VOID };

// Symbol table entry for a variable
struct VarInfo {
    std::string name;
    ValueType type;
    // Runtime storage index (index in interpreter's memory vector)
    size_t memIndex;
};

// Intermediate code instruction types
enum class OpCode {
    // Arithmetic and logical operations
    ADD, SUB, MUL, DIV, MOD,
    LT, LE, GT, GE, EQ, NE,        // comparisons
    AND, OR, NOT,                 // logical ops
    // Data operations
    PUSH_INT, PUSH_REAL, PUSH_BOOL, PUSH_STRING,  // push constants
    PUSH_VAR,                     // push variable value
    PUSH_ADDR,                    // push address of variable (for assignment/read)
    ASSIGN,                       // assign value to variable (pops address and value)
    // Control flow
    JMP,                          // unconditional jump
    JMP_IF_FALSE,                 // jump if false (pop condition)
    // I/O operations
    READ, WRITE,                  // input and output
    // No-op (used for placeholders or labels)
    NOP
};

// Structure for an intermediate code instruction (POLIZ element)
struct Instruction {
    OpCode op;
    // Operand fields (only one of these is used depending on op type)
    int intValue;
    double realValue;
    bool boolValue;
    std::string stringValue;
    size_t index;   // index for variables (symbol table or jump target index)

    Instruction(OpCode opc = OpCode::NOP) : op(opc), intValue(0), realValue(0.0), boolValue(false), index(0) {}
};

// Parser/semantic analyzer class
class Parser {
public:
    Parser(Lexer& lex): lexer(lex), errorFlag(false) {}

    // Parse the entire program. Returns true if success, false if error.
    bool parseProgram();

    // Get the generated intermediate code and symbol table (for interpreter)
    const std::vector<Instruction>& getCode() const { return code; }
    const std::vector<VarInfo>& getSymbolTable() const { return variables; }

private:
    Lexer& lexer;
    Token currentToken;
    bool errorFlag;

    // Intermediate code (POLIZ) and symbol table
    std::vector<Instruction> code;
    std::vector<VarInfo> variables;
    // Map for variable name -> symbol table index
    std::unordered_map<std::string, size_t> varIndexMap;

    // Label management for goto and loop/case control flow
    struct LabelInfo { size_t index; bool defined; std::vector<size_t> patchLocations; };
    std::unordered_map<std::string, LabelInfo> labelMap;

    // Structures to manage break/continue targets in nested loops and switch-case
    struct LoopContext { std::string breakLabel; std::string continueLabel; };
    std::vector<LoopContext> loopStack;
    struct SwitchContext { std::string breakLabel; };
    std::vector<SwitchContext> switchStack;

    // Parsing functions for each grammar construct
    void advance();              // consume current token and move to next
    void expect(TokenType expectedType);  // expect current token to be of a certain type or flag error

    void parseDeclarations();
    void parseStatement();
    void parseCompoundStatement();
    void parseIfStatement();
    void parseWhileStatement();
    void parseDoWhileStatement();
    void parseForStatement();
    void parseCaseStatement();   // switch-case
    void parseGotoStatement();
    void parseReadStatement();
    void parseWriteStatement();
    void parseBreakStatement();
    void parseContinueStatement();

    // Expression parsing (recursive descent by precedence)
    ValueType parseExpression();          // parse assignment expression (includes '=' operator)
    ValueType parseLogicalOrExpression();
    ValueType parseLogicalAndExpression();
    ValueType parseEqualityExpression();
    ValueType parseRelationalExpression();
    ValueType parseAdditiveExpression();
    ValueType parseMultiplicativeExpression();
    ValueType parseUnaryExpression();
    ValueType parsePrimaryExpression();

    // Utility methods for semantic actions and code generation
    size_t addVariable(const std::string& name, ValueType type);
    size_t getVariableIndex(const std::string& name);
    void emit(const Instruction& instr);                // append an instruction
    void emitJump(OpCode op, size_t targetIndexPlaceholder); // emit jump with placeholder index
    void backpatch(size_t instrIndex, size_t targetIndex);   // patch a jump target in code
    // Error handling
    void semanticError(const std::string& msg);
};

#endif // PARSER_H
