#include "executor.h"
#include "ident.h"
#include <iostream>
#include <stack>
#include <stdexcept>
#include <cmath>

void Executer::execute(const std::vector<Lex> &poliz) {
    std::stack<Value> st;  // evaluation stack
    for (size_t index = 0; index < poliz.size(); ++index) {
        Lex lex = poliz[index];
        LexType type = lex.getType();
        switch (type) {
            // Push constants and addresses onto stack
            case LEX_TRUE:
            case LEX_FALSE:
            case LEX_INT_CONST:
            case POLIZ_ADDRESS:
            case POLIZ_LABEL:
                st.push(lex.getValue());
                break;
            case LEX_REAL_CONST: {
                double dval = real_const_table[lex.getValue()];
                st.push(dval);
                break;
            }
            case LEX_STR_CONST: {
                const std::string &sval = string_const_table[lex.getValue()];
                st.push(sval);
                break;
            }
            case LEX_ID: {
                // Push variable value
                int idx = lex.getValue();
                if (!TID[idx].assign) {
                    throw std::runtime_error("POLIZ: indefinite identifier");
                }
                LexType vtype = TID[idx].type;
                if (vtype == LEX_INT) {
                    // int stored as int
                    int ival = 0;
                    if (std::holds_alternative<int>(TID[idx].value)) {
                        ival = std::get<int>(TID[idx].value);
                    } else if (std::holds_alternative<double>(TID[idx].value)) {
                        ival = static_cast<int>(std::get<double>(TID[idx].value));
                    } else if (std::holds_alternative<bool>(TID[idx].value)) {
                        ival = std::get<bool>(TID[idx].value) ? 1 : 0;
                    }
                    st.push(ival);
                } else if (vtype == LEX_REAL) {
                    double dval = 0.0;
                    if (std::holds_alternative<double>(TID[idx].value)) {
                        dval = std::get<double>(TID[idx].value);
                    } else if (std::holds_alternative<int>(TID[idx].value)) {
                        dval = static_cast<double>(std::get<int>(TID[idx].value));
                    } else if (std::holds_alternative<bool>(TID[idx].value)) {
                        dval = std::get<bool>(TID[idx].value) ? 1.0 : 0.0;
                    }
                    st.push(dval);
                } else if (vtype == LEX_BOOL) {
                    bool bval = false;
                    if (std::holds_alternative<bool>(TID[idx].value)) {
                        bval = std::get<bool>(TID[idx].value);
                    } else if (std::holds_alternative<int>(TID[idx].value)) {
                        bval = (std::get<int>(TID[idx].value) != 0);
                    }
                    // push as int (0 or 1)
                    st.push(bval ? 1 : 0);
                } else if (vtype == LEX_STRING) {
                    const std::string &sval = std::get<std::string>(TID[idx].value);
                    st.push(sval);
                }
                break;
            }
            // Logical NOT
            case LEX_NOT: {
                if (st.empty()) throw std::runtime_error("POLIZ: stack underflow on NOT");
                int i = std::get<int>(st.top()); st.pop();
                st.push((!i) ? 1 : 0);
                break;
            }
            // Logical OR
            case LEX_OR: {
                if (st.size() < 2) throw std::runtime_error("POLIZ: stack underflow on OR");
                int r = std::get<int>(st.top()); st.pop();
                int l = std::get<int>(st.top()); st.pop();
                st.push((l || r) ? 1 : 0);
                break;
            }
            // Logical AND
            case LEX_AND: {
                if (st.size() < 2) throw std::runtime_error("POLIZ: stack underflow on AND");
                int r = std::get<int>(st.top()); st.pop();
                int l = std::get<int>(st.top()); st.pop();
                st.push((l && r) ? 1 : 0);
                break;
            }
            // Addition (numeric or string concatenation)
            case LEX_PLUS: {
                if (st.size() < 2) throw std::runtime_error("POLIZ: stack underflow on '+'");
                Value v1 = st.top(); st.pop();
                Value v2 = st.top(); st.pop();
                if (std::holds_alternative<std::string>(v1) || std::holds_alternative<std::string>(v2)) {
                    // String concatenation
                    std::string s1 = std::holds_alternative<std::string>(v2) ? std::get<std::string>(v2)
                                                                             : std::to_string(std::get<int>(v2));
                    std::string s2 = std::holds_alternative<std::string>(v1) ? std::get<std::string>(v1)
                                                                             : std::to_string(std::get<int>(v1));
                    st.push(s1 + s2);
                } else {
                    // Numeric addition
                    double a = std::holds_alternative<double>(v1) ? std::get<double>(v1)
                                                                  : (double) std::get<int>(v1);
                    double b = std::holds_alternative<double>(v2) ? std::get<double>(v2)
                                                                  : (double) std::get<int>(v2);
                    if (std::holds_alternative<double>(v1) || std::holds_alternative<double>(v2)) {
                        st.push(b + a);
                    } else {
                        // both int
                        st.push((int)std::get<int>(v2) + std::get<int>(v1));
                    }
                }
                break;
            }
            // Subtraction
            case LEX_MINUS: {
                if (st.size() < 2) throw std::runtime_error("POLIZ: stack underflow on '-'");
                Value v1 = st.top(); st.pop();
                Value v2 = st.top(); st.pop();
                double a = std::holds_alternative<double>(v1) ? std::get<double>(v1)
                                                              : (double) std::get<int>(v1);
                double b = std::holds_alternative<double>(v2) ? std::get<double>(v2)
                                                              : (double) std::get<int>(v2);
                if (std::holds_alternative<double>(v1) || std::holds_alternative<double>(v2)) {
                    st.push(b - a);
                } else {
                    st.push(std::get<int>(v2) - std::get<int>(v1));
                }
                break;
            }
            // Multiplication
            case LEX_TIMES: {
                if (st.size() < 2) throw std::runtime_error("POLIZ: stack underflow on '*'");
                Value v1 = st.top(); st.pop();
                Value v2 = st.top(); st.pop();
                double a = std::holds_alternative<double>(v1) ? std::get<double>(v1)
                                                              : (double) std::get<int>(v1);
                double b = std::holds_alternative<double>(v2) ? std::get<double>(v2)
                                                              : (double) std::get<int>(v2);
                if (std::holds_alternative<double>(v1) || std::holds_alternative<double>(v2)) {
                    st.push(b * a);
                } else {
                    st.push(std::get<int>(v2) * std::get<int>(v1));
                }
                break;
            }
            // Division
            case LEX_SLASH: {
                if (st.size() < 2) throw std::runtime_error("POLIZ: stack underflow on '/'");
                Value v1 = st.top(); st.pop();
                Value v2 = st.top(); st.pop();
                // Check division by zero
                if ((std::holds_alternative<int>(v1) && std::get<int>(v1) == 0) ||
                    (std::holds_alternative<double>(v1) && fabs(std::get<double>(v1)) < 1e-12)) {
                    throw std::runtime_error("POLIZ: divide by zero");
                }
                double a = std::holds_alternative<double>(v1) ? std::get<double>(v1)
                                                              : (double) std::get<int>(v1);
                double b = std::holds_alternative<double>(v2) ? std::get<double>(v2)
                                                              : (double) std::get<int>(v2);
                if (std::holds_alternative<double>(v1) || std::holds_alternative<double>(v2)) {
                    st.push(b / a);
                } else {
                    st.push(std::get<int>(v2) / std::get<int>(v1));
                }
                break;
            }
            // Modulus
            case LEX_PERCENT: {
                if (st.size() < 2) throw std::runtime_error("POLIZ: stack underflow on '%'");
                int r = std::get<int>(st.top()); st.pop();
                int l = std::get<int>(st.top()); st.pop();
                if (r == 0) throw std::runtime_error("POLIZ: divide by zero");
                st.push(l % r);
                break;
            }
            // Equality
            case LEX_EQ: {
                if (st.size() < 2) throw std::runtime_error("POLIZ: stack underflow on '=='");
                Value v1 = st.top(); st.pop();
                Value v2 = st.top(); st.pop();
                int result;
                if (std::holds_alternative<std::string>(v1) && std::holds_alternative<std::string>(v2)) {
                    result = (std::get<std::string>(v2) == std::get<std::string>(v1)) ? 1 : 0;
                } else if (std::holds_alternative<double>(v1) || std::holds_alternative<double>(v2)) {
                    double a = std::holds_alternative<double>(v1) ? std::get<double>(v1)
                                                                  : (double) std::get<int>(v1);
                    double b = std::holds_alternative<double>(v2) ? std::get<double>(v2)
                                                                  : (double) std::get<int>(v2);
                    result = (fabs(b - a) < 1e-12) ? 1 : 0;
                } else {
                    // int vs int (also covers bool represented as int)
                    result = (std::get<int>(v2) == std::get<int>(v1)) ? 1 : 0;
                }
                st.push(result);
                break;
            }
            // Inequality
            case LEX_NEQ: {
                if (st.size() < 2) throw std::runtime_error("POLIZ: stack underflow on '!='");
                Value v1 = st.top(); st.pop();
                Value v2 = st.top(); st.pop();
                int result;
                if (std::holds_alternative<std::string>(v1) && std::holds_alternative<std::string>(v2)) {
                    result = (std::get<std::string>(v2) != std::get<std::string>(v1)) ? 1 : 0;
                } else if (std::holds_alternative<double>(v1) || std::holds_alternative<double>(v2)) {
                    double a = std::holds_alternative<double>(v1) ? std::get<double>(v1)
                                                                  : (double) std::get<int>(v1);
                    double b = std::holds_alternative<double>(v2) ? std::get<double>(v2)
                                                                  : (double) std::get<int>(v2);
                    result = (fabs(b - a) >= 1e-12) ? 1 : 0;
                } else {
                    result = (std::get<int>(v2) != std::get<int>(v1)) ? 1 : 0;
                }
                st.push(result);
                break;
            }
            // Less-than
            case LEX_LESS: {
                if (st.size() < 2) throw std::runtime_error("POLIZ: stack underflow on '<'");
                Value v1 = st.top(); st.pop();
                Value v2 = st.top(); st.pop();
                double a = std::holds_alternative<double>(v1) ? std::get<double>(v1)
                                                              : (double) std::get<int>(v1);
                double b = std::holds_alternative<double>(v2) ? std::get<double>(v2)
                                                              : (double) std::get<int>(v2);
                st.push((b < a) ? 1 : 0);
                break;
            }
            // Greater-or-equal
            case LEX_GEQ: {
                if (st.size() < 2) throw std::runtime_error("POLIZ: stack underflow on '>='");
                Value v1 = st.top(); st.pop();
                Value v2 = st.top(); st.pop();
                double a = std::holds_alternative<double>(v1) ? std::get<double>(v1)
                                                              : (double) std::get<int>(v1);
                double b = std::holds_alternative<double>(v2) ? std::get<double>(v2)
                                                              : (double) std::get<int>(v2);
                st.push((b >= a) ? 1 : 0);
                break;
            }
            // Greater-than
            case LEX_GREATER: {
                if (st.size() < 2) throw std::runtime_error("POLIZ: stack underflow on '>'");
                Value v1 = st.top(); st.pop();
                Value v2 = st.top(); st.pop();
                double a = std::holds_alternative<double>(v1) ? std::get<double>(v1)
                                                              : (double) std::get<int>(v1);
                double b = std::holds_alternative<double>(v2) ? std::get<double>(v2)
                                                              : (double) std::get<int>(v2);
                st.push((b > a) ? 1 : 0);
                break;
            }
            // Less-or-equal
            case LEX_LEQ: {
                if (st.size() < 2) throw std::runtime_error("POLIZ: stack underflow on '<='");
                Value v1 = st.top(); st.pop();
                Value v2 = st.top(); st.pop();
                double a = std::holds_alternative<double>(v1) ? std::get<double>(v1)
                                                              : (double) std::get<int>(v1);
                double b = std::holds_alternative<double>(v2) ? std::get<double>(v2)
                                                              : (double) std::get<int>(v2);
                st.push((b <= a) ? 1 : 0);
                break;
            }
            // Assignment
            case LEX_ASSIGN: {
                if (st.size() < 2) throw std::runtime_error("POLIZ: stack underflow on '='");
                Value val = st.top(); st.pop();
                int idx = std::get<int>(st.top()); st.pop();
                LexType varType = TID[idx].type;
                Value assignedVal;
                if (varType == LEX_INT) {
                    int ival;
                    if (std::holds_alternative<double>(val)) {
                        ival = (int) std::get<double>(val);
                    } else if (std::holds_alternative<int>(val)) {
                        ival = std::get<int>(val);
                    } else if (std::holds_alternative<bool>(val)) {
                        ival = std::get<bool>(val) ? 1 : 0;
                    } else {
                        throw std::runtime_error("Type mismatch in int assignment");
                    }
                    TID[idx].value = ival;
                    assignedVal = ival;
                } else if (varType == LEX_REAL) {
                    double dval;
                    if (std::holds_alternative<double>(val)) {
                        dval = std::get<double>(val);
                    } else if (std::holds_alternative<int>(val)) {
                        dval = (double) std::get<int>(val);
                    } else if (std::holds_alternative<bool>(val)) {
                        dval = std::get<bool>(val) ? 1.0 : 0.0;
                    } else {
                        throw std::runtime_error("Type mismatch in real assignment");
                    }
                    TID[idx].value = dval;
                    assignedVal = dval;
                } else if (varType == LEX_BOOL) {
                    bool bval;
                    if (std::holds_alternative<int>(val)) {
                        bval = (std::get<int>(val) != 0);
                    } else if (std::holds_alternative<bool>(val)) {
                        bval = std::get<bool>(val);
                    } else {
                        throw std::runtime_error("Type mismatch in bool assignment");
                    }
                    TID[idx].value = bval;
                    assignedVal = (bval ? 1 : 0);
                } else if (varType == LEX_STRING) {
                    if (!std::holds_alternative<std::string>(val)) {
                        throw std::runtime_error("Type mismatch in string assignment");
                    }
                    std::string sval = std::get<std::string>(val);
                    TID[idx].value = sval;
                    assignedVal = sval;
                } else {
                    throw std::runtime_error("Unknown variable type in assignment");
                }
                TID[idx].assign = true;
                // Push the assigned value as the result of the assignment expression
                st.push(assignedVal);
                break;
            }
            // Read input into a variable
            case LEX_READ: {
                if (st.empty()) throw std::runtime_error("POLIZ: stack underflow on READ");
                int idx = std::get<int>(st.top()); st.pop();
                LexType vtype = TID[idx].type;
                if (vtype == LEX_INT) {
                    int ival;
                    if (!(std::cin >> ival)) throw std::runtime_error("Input error while reading int");
                    TID[idx].value = ival;
                } else if (vtype == LEX_REAL) {
                    double dval;
                    if (!(std::cin >> dval)) throw std::runtime_error("Input error while reading real");
                    TID[idx].value = dval;
                } else if (vtype == LEX_BOOL) {
                    std::string token;
                    if (!(std::cin >> token)) throw std::runtime_error("Input error while reading bool");
                    bool bval;
                    if (token == "true" || token == "1") bval = true;
                    else if (token == "false" || token == "0") bval = false;
                    else throw std::runtime_error("Invalid boolean input");
                    TID[idx].value = bval;
                } else if (vtype == LEX_STRING) {
                    std::string sval;
                    if (!(std::cin >> sval)) throw std::runtime_error("Input error while reading string");
                    TID[idx].value = sval;
                } else {
                    throw std::runtime_error("Cannot read into this type");
                }
                TID[idx].assign = true;
                break;
            }
            // Write output
            case LEX_WRITE: {
                if (st.empty()) throw std::runtime_error("POLIZ: stack underflow on WRITE");
                Value val = st.top(); st.pop();
                if (std::holds_alternative<int>(val)) {
                    std::cout << std::get<int>(val);
                } else if (std::holds_alternative<double>(val)) {
                    std::cout << std::get<double>(val);
                } else if (std::holds_alternative<std::string>(val)) {
                    std::cout << std::get<std::string>(val);
                } else if (std::holds_alternative<bool>(val)) {
                    std::cout << (std::get<bool>(val) ? 1 : 0);
                }
                std::cout << std::endl;
                break;
            }
            // Unconditional jump
            case POLIZ_GO: {
                if (st.empty()) throw std::runtime_error("POLIZ: stack underflow on GO");
                int addr = std::get<int>(st.top()); st.pop();
                index = addr - 1;  // -1 to compensate for automatic index increment
                break;
            }
            // Conditional jump (jump if false)
            case POLIZ_FGO: {
                if (st.size() < 2) throw std::runtime_error("POLIZ: stack underflow on FGO");
                int addr = std::get<int>(st.top()); st.pop();
                int cond = std::get<int>(st.top()); st.pop();
                if (!cond) {
                    index = addr - 1;
                }
                break;
            }
            default:
                throw std::runtime_error("POLIZ: unexpected element");
        }
    }
}
