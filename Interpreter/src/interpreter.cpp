#include "interpreter.h"
#include <iomanip>

void Interpreter::run() {
    size_t pc = 0;  // program counter (index in code vector)
    bool running = true;
    std::cout << "[Execution] Starting program execution...\n";
    while (running && pc < code.size()) {
        const Instruction& instr = code[pc];
        switch (instr.op) {
            case OpCode::PUSH_INT: {
                Value v; v.type = ValueType::INT; v.i = instr.intValue;
                pushValue(v);
                break;
            }
            case OpCode::PUSH_REAL: {
                Value v; v.type = ValueType::REAL; v.r = instr.realValue;
                pushValue(v);
                break;
            }
            case OpCode::PUSH_BOOL: {
                Value v; v.type = ValueType::BOOL; v.b = instr.boolValue;
                pushValue(v);
                break;
            }
            case OpCode::PUSH_STRING: {
                Value v; v.type = ValueType::STRING; v.s = instr.stringValue;
                pushValue(v);
                break;
            }
            case OpCode::PUSH_VAR: {
                // Push value of variable at index
                size_t idx = instr.index;
                Value v = memory[idx];
                pushValue(v);
                break;
            }
            case OpCode::PUSH_ADDR: {
                // Push an address (we represent address as an integer index in the Value)
                Value v; v.type = ValueType::INT; v.i = instr.index;
                pushValue(v);
                break;
            }
            case OpCode::ASSIGN: {
                // Pop value and address, assign and push the value back
                Value val = popValue();
                Value adrV = popValue();
                size_t adr = (size_t)adrV.i;
                // Store into memory
                ValueType dstT = memory[adr].type;
                Value res = val;
                switch (dstT) {
                    case ValueType::INT:
                        if (val.type == ValueType::REAL)   res.i = (long long)val.r;
                        else if (val.type == ValueType::BOOL) res.i = val.b ? 1 : 0;
                        res.type = ValueType::INT;
                        break;
                    case ValueType::REAL:
                        if (val.type == ValueType::INT)    res.r = (double)val.i;
                        else if (val.type == ValueType::BOOL) res.r = val.b ? 1.0 : 0.0;
                        res.type = ValueType::REAL;
                        break;
                    case ValueType::BOOL:
                        if (val.type == ValueType::REAL)   res.b = (val.r != 0.0);
                        else if (val.type == ValueType::INT)  res.b = (val.i != 0);
                        res.type = ValueType::BOOL;
                        break;
                    case ValueType::STRING:
                        /* для строки ничего переводить не нужно */
                        res.type = ValueType::STRING;
                        break;
                    default:
                        /* если переменная ещё не инициализирована – разрешаем любой тип */
                        dstT = res.type;
                }
                /* === записываем и оставляем значение на стеке === */
                memory[adr] = res;
                pushValue(res);
                break;
            }
            case OpCode::ADD: {
                Value v2 = popValue();
                Value v1 = popValue();
                // Numeric addition or string concatenation
                if (v1.type == ValueType::STRING && v2.type == ValueType::STRING) {
                    Value res; res.type = ValueType::STRING;
                    res.s = v1.s + v2.s;
                    pushValue(res);
                } else {
                    // Promote types if needed
                    double a = 0, b = 0;
                    if (v1.type == ValueType::REAL) a = v1.r;
                    else a = (double) v1.i;
                    if (v2.type == ValueType::REAL) b = v2.r;
                    else b = (double) v2.i;
                    double sum = a + b;
                    Value res;
                    if (v1.type == ValueType::REAL || v2.type == ValueType::REAL) {
                        res.type = ValueType::REAL;
                        res.r = sum;
                    } else {
                        res.type = ValueType::INT;
                        res.i = (long long) sum;
                    }
                    pushValue(res);
                }
                break;
            }
            case OpCode::SUB: {
                Value v2 = popValue();
                Value v1 = popValue();
                double a = (v1.type == ValueType::REAL ? v1.r : (double)v1.i);
                double b = (v2.type == ValueType::REAL ? v2.r : (double)v2.i);
                double diff = a - b;
                Value res;
                if (v1.type == ValueType::REAL || v2.type == ValueType::REAL) {
                    res.type = ValueType::REAL;
                    res.r = diff;
                } else {
                    res.type = ValueType::INT;
                    res.i = (long long) diff;
                }
                pushValue(res);
                break;
            }
            case OpCode::MUL: {
                Value v2 = popValue();
                Value v1 = popValue();
                double a = (v1.type == ValueType::REAL ? v1.r : (double)v1.i);
                double b = (v2.type == ValueType::REAL ? v2.r : (double)v2.i);
                double prod = a * b;
                Value res;
                if (v1.type == ValueType::REAL || v2.type == ValueType::REAL) {
                    res.type = ValueType::REAL;
                    res.r = prod;
                } else {
                    res.type = ValueType::INT;
                    res.i = (long long) prod;
                }
                pushValue(res);
                break;
            }
            case OpCode::DIV: {
                Value v2 = popValue();
                Value v1 = popValue();
                double a = (v1.type == ValueType::REAL ? v1.r : (double)v1.i);
                double b = (v2.type == ValueType::REAL ? v2.r : (double)v2.i);
                if (b == 0) {
                    std::cerr << "Runtime error: Division by zero\n";
                    b = 1; // avoid crash, continue with dummy value
                }
                double quotient = a / b;
                Value res;
                if (v1.type == ValueType::REAL || v2.type == ValueType::REAL) {
                    res.type = ValueType::REAL;
                    res.r = quotient;
                } else {
                    res.type = ValueType::INT;
                    res.i = (long long) quotient;
                }
                pushValue(res);
                break;
            }
            case OpCode::MOD: {
                Value v2 = popValue();
                Value v1 = popValue();
                long long a = (v1.type == ValueType::INT ? v1.i : (long long)v1.r);
                long long b = (v2.type == ValueType::INT ? v2.i : (long long)v2.r);
                if (b == 0) {
                    std::cerr << "Runtime error: Modulo by zero\n";
                    b = 1;
                }
                Value res; res.type = ValueType::INT;
                res.i = a % b;
                pushValue(res);
                break;
            }
            case OpCode::LT: {
                Value v2 = popValue();
                Value v1 = popValue();
                // Support numeric and string comparison
                bool result;
                if (v1.type == ValueType::STRING && v2.type == ValueType::STRING) {
                    result = (v1.s < v2.s);
                } else {
                    double a = (v1.type == ValueType::REAL ? v1.r : (double)v1.i);
                    double b = (v2.type == ValueType::REAL ? v2.r : (double)v2.i);
                    result = (a < b);
                }
                Value res; res.type = ValueType::BOOL; res.b = result;
                pushValue(res);
                break;
            }
            case OpCode::LE: {
                Value v2 = popValue();
                Value v1 = popValue();
                bool result;
                if (v1.type == ValueType::STRING && v2.type == ValueType::STRING) {
                    result = (v1.s <= v2.s);
                } else {
                    double a = (v1.type == ValueType::REAL ? v1.r : (double)v1.i);
                    double b = (v2.type == ValueType::REAL ? v2.r : (double)v2.i);
                    result = (a <= b);
                }
                Value res; res.type = ValueType::BOOL; res.b = result;
                pushValue(res);
                break;
            }
            case OpCode::GT: {
                Value v2 = popValue();
                Value v1 = popValue();
                bool result;
                if (v1.type == ValueType::STRING && v2.type == ValueType::STRING) {
                    result = (v1.s > v2.s);
                } else {
                    double a = (v1.type == ValueType::REAL ? v1.r : (double)v1.i);
                    double b = (v2.type == ValueType::REAL ? v2.r : (double)v2.i);
                    result = (a > b);
                }
                Value res; res.type = ValueType::BOOL; res.b = result;
                pushValue(res);
                break;
            }
            case OpCode::GE: {
                Value v2 = popValue();
                Value v1 = popValue();
                bool result;
                if (v1.type == ValueType::STRING && v2.type == ValueType::STRING) {
                    result = (v1.s >= v2.s);
                } else {
                    double a = (v1.type == ValueType::REAL ? v1.r : (double)v1.i);
                    double b = (v2.type == ValueType::REAL ? v2.r : (double)v2.i);
                    result = (a >= b);
                }
                Value res; res.type = ValueType::BOOL; res.b = result;
                pushValue(res);
                break;
            }
            case OpCode::EQ: {
                Value v2 = popValue();
                Value v1 = popValue();
                bool result;
                if (v1.type == ValueType::STRING && v2.type == ValueType::STRING) {
                    result = (v1.s == v2.s);
                } else if (v1.type == ValueType::BOOL && v2.type == ValueType::BOOL) {
                    result = (v1.b == v2.b);
                } else {
                    double a = (v1.type == ValueType::REAL ? v1.r : (double)v1.i);
                    double b = (v2.type == ValueType::REAL ? v2.r : (double)v2.i);
                    result = (a == b);
                }
                Value res; res.type = ValueType::BOOL; res.b = result;
                pushValue(res);
                break;
            }
            case OpCode::NE: {
                Value v2 = popValue();
                Value v1 = popValue();
                bool result;
                if (v1.type == ValueType::STRING && v2.type == ValueType::STRING) {
                    result = (v1.s != v2.s);
                } else if (v1.type == ValueType::BOOL && v2.type == ValueType::BOOL) {
                    result = (v1.b != v2.b);
                } else {
                    double a = (v1.type == ValueType::REAL ? v1.r : (double)v1.i);
                    double b = (v2.type == ValueType::REAL ? v2.r : (double)v2.i);
                    result = (a != b);
                }
                Value res; res.type = ValueType::BOOL; res.b = result;
                pushValue(res);
                break;
            }
            case OpCode::AND: {
                Value v2 = popValue();
                Value v1 = popValue();
                bool result = ( (v1.type == ValueType::BOOL ? v1.b : v1.i != 0) &&
                                (v2.type == ValueType::BOOL ? v2.b : v2.i != 0) );
                Value res; res.type = ValueType::BOOL; res.b = result;
                pushValue(res);
                break;
            }
            case OpCode::OR: {
                Value v2 = popValue();
                Value v1 = popValue();
                bool result = ( (v1.type == ValueType::BOOL ? v1.b : v1.i != 0) ||
                                (v2.type == ValueType::BOOL ? v2.b : v2.i != 0) );
                Value res; res.type = ValueType::BOOL; res.b = result;
                pushValue(res);
                break;
            }
            case OpCode::NOT: {
                Value v = popValue();
                bool result = !(v.type == ValueType::BOOL ? v.b : v.i != 0);
                Value res; res.type = ValueType::BOOL; res.b = result;
                pushValue(res);
                break;
            }
            case OpCode::JMP: {
                // Unconditional jump
                pc = instr.index;
                continue; // jump to next iteration without pc++ to avoid auto-increment
            }
            case OpCode::JMP_IF_FALSE: {
                Value cond = popValue();
                bool condition = (cond.type == ValueType::BOOL ? cond.b : cond.i != 0);
                if (!condition) {
                    pc = instr.index;
                    continue;
                }
                break;
            }
            case OpCode::READ: {
                // Pop address and read into that memory location
                Value addrVal = popValue();
                size_t addr = (size_t) addrVal.i;
                ValueType varType = memory[addr].type;
                if (varType == ValueType::INT) {
                    long long inVal;
                    std::cin >> inVal;
                    memory[addr].i = inVal;
                } else if (varType == ValueType::REAL) {
                    double inVal;
                    std::cin >> inVal;
                    memory[addr].r = inVal;
                } else if (varType == ValueType::BOOL) {
                    std::string token;
                    std::cin >> token;
                    // Accept "true"/"false" or "1"/"0"
                    bool val = false;
                    if (token == "true" || token == "1") val = true;
                    else if (token == "false" || token == "0") val = false;
                    else std::cerr << "Runtime warning: unrecognized bool input, defaulting to false\n";
                    memory[addr].b = val;
                } else if (varType == ValueType::STRING) {
                    std::string inStr;
                    // Read entire line (after any whitespace)
                    std::getline(std::cin, inStr);
                    if (inStr.size() == 0) {
                        // if newline was immediately after previous token, get actual line
                        std::getline(std::cin, inStr);
                    }
                    memory[addr].s = inStr;
                }
                break;
            }
            case OpCode::WRITE: {
                // Pop value and output it
                Value val = popValue();
                switch(val.type) {
                    case ValueType::INT:
                        std::cout << val.i;
                        break;
                    case ValueType::REAL:
                        // Print real with no trailing zeros
                        std::cout << std::fixed << std::setprecision(6) << val.r;
                        break;
                    case ValueType::BOOL:
                        std::cout << (val.b ? "true" : "false");
                        break;
                    case ValueType::STRING:
                        std::cout << val.s;
                        break;
                    default:
                        break;
                }
                std::cout << std::endl;
                break;
            }
            case OpCode::NOP:
            default:
                // NOP or unrecognized op: do nothing
                break;
        }
        pc++;
    }
    std::cout << "[Execution] Program execution finished.\n";
}
