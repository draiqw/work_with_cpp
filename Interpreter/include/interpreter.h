#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"
#include <vector>
#include <string>
#include <iostream>

// A union-like structure to hold a runtime value (int, real, bool or string)
struct Value {
    ValueType type;
    long long i;
    double r;
    bool b;
    std::string s;

    Value(): type(ValueType::VOID), i(0), r(0.0), b(false) {}
};

class Interpreter {
public:
    Interpreter(const std::vector<Instruction>& code,
                const std::vector<VarInfo>& vars)
        : code(code), variables(vars) {
        // allocate memory for variables
        memory.resize(variables.size());
        // initialize default values
        for (size_t idx = 0; idx < variables.size(); ++idx) {
            memory[idx].type = variables[idx].type;
            // default initialize based on type
            if (memory[idx].type == ValueType::INT) memory[idx].i = 0;
            if (memory[idx].type == ValueType::REAL) memory[idx].r = 0.0;
            if (memory[idx].type == ValueType::BOOL) memory[idx].b = false;
            if (memory[idx].type == ValueType::STRING) memory[idx].s = "";
        }
    }

    // Run the interpreter (execute the POLIZ code)
    void run();

private:
    const std::vector<Instruction>& code;
    const std::vector<VarInfo>& variables;
    std::vector<Value> memory;    // storage for variables (by index)
    std::vector<Value> evalStack; // evaluation stack for executing code

    // Helper for pushing a value onto evalStack
    void pushValue(const Value& val) { evalStack.push_back(val); }

    // Pop top of evalStack
    Value popValue() {
        Value val = evalStack.back();
        evalStack.pop_back();
        return val;
    }
};

#endif // INTERPRETER_H
