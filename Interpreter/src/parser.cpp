#include "parser.h"
#include <stdexcept>
#include <iostream>

void Parser::analyze() {
    gl();
    parseProgram();
    if (curr_type != LEX_FIN) {
        parseError("Unexpected tokens after end of program");
    }
}

// <program> -> program { <block> }
void Parser::parseProgram() {
    if (curr_type != LEX_PROGRAM) {
        parseError("Program must start with 'program'");
    }
    gl();
    if (curr_type != LEX_LBRACE) {
        parseError("Missing '{' after 'program'");
    }
    gl();
    parseBlock();
    if (curr_type != LEX_RBRACE) {
        parseError("Missing '}' at end of program");
    }
    gl();
}

// Parse a block: declarations followed by statements until a closing brace
void Parser::parseBlock() {
    // Parse declarations at block start
    while (curr_type == LEX_INT || curr_type == LEX_BOOL || curr_type == LEX_REAL || curr_type == LEX_STRING) {
        parseDeclaration();
    }
    // Parse statements until end-of-block
    while (curr_type != LEX_RBRACE && curr_type != LEX_FIN) {
        parseStatement();
    }
}

// <declaration> -> (int|bool|real|string) Ident [ = <expression> ] { , Ident [ = <expression> ] } ;
void Parser::parseDeclaration() {
    // Determine data type being declared
    DataType dtype;
    switch (curr_type) {
        case LEX_INT:    dtype = D_INT; break;
        case LEX_BOOL:   dtype = D_BOOL; break;
        case LEX_REAL:   dtype = D_REAL; break;
        case LEX_STRING: dtype = D_STRING; break;
        default:
            parseError("Unknown type in declaration");
    }
    gl();
    if (curr_type != LEX_ID) {
        parseError("Expected identifier after type");
    }
    while (true) {
        int id_index = curr_val;
        std::string name = TID[id_index].name;
        if (TID[id_index].declare) {
            parseError("Variable '" + name + "' already declared");
        }
        if (label_map.count(name) || label_fixups.count(name)) {
            parseError("Identifier '" + name + "' conflicts with a label");
        }
        // Mark identifier as declared with this type
        TID[id_index].declare = true;
        TID[id_index].type = (dtype == D_INT ? LEX_INT :
                               dtype == D_BOOL ? LEX_BOOL :
                               dtype == D_REAL ? LEX_REAL : LEX_STRING);
        // Optional initializer
        gl();
        if (curr_type == LEX_ASSIGN) {
            gl();
            DataType expr_type = parseExpression();
            // Type compatibility check
            if (dtype == D_BOOL && expr_type != D_BOOL) {
                parseError("Cannot assign non-bool to bool variable '" + name + "'");
            }
            if (dtype == D_STRING && expr_type != D_STRING) {
                parseError("Cannot assign non-string to string variable '" + name + "'");
            }
            if ((dtype == D_INT || dtype == D_REAL) && !(expr_type == D_INT || expr_type == D_REAL)) {
                parseError("Cannot assign non-numeric to numeric variable '" + name + "'");
            }
            // Generate runtime initialization code: address, value, assign
            emit(Lex(POLIZ_ADDRESS, id_index));
            // (expression code already emitted onto poliz by parseExpression)
            emit(Lex(LEX_ASSIGN));
        }
        // If multiple identifiers separated by comma
        if (curr_type == LEX_COMMA) {
            gl();
            if (curr_type != LEX_ID) {
                parseError("Expected identifier after ','");
            }
            continue;  // loop to process the next identifier in the list
        }
        // Must end the declaration with semicolon
        if (curr_type != LEX_SEMICOLON) {
            parseError("Missing ';' after variable declaration");
        }
        gl();
        break;
    }
}

// Parse a single statement (not a declaration)
void Parser::parseStatement() {
    switch (curr_type) {
        case LEX_IF: {
            // if (<condition>) <statement> [ else <statement> ]
            gl();
            if (curr_type != LEX_LPAREN) parseError("Expected '(' after 'if'");
            gl();
            DataType cond_type = parseExpression();
            if (cond_type != D_BOOL) parseError("Condition in 'if' must be boolean");
            if (curr_type != LEX_RPAREN) parseError("Expected ')' after if condition");
            gl();
            // Reserve place for jump if false
            int pl_fgo = poliz.size();
            emit(Lex());              // placeholder for jump address
            emit(Lex(POLIZ_FGO));
            // Parse 'then' branch
            parseStatement();
            if (curr_type == LEX_ELSE) {
                // Reserve place for jump over else branch
                int pl_go = poliz.size();
                emit(Lex());
                emit(Lex(POLIZ_GO));
                // Patch false jump to here (start of else)
                poliz[pl_fgo] = Lex(POLIZ_LABEL, poliz.size());
                gl();
                parseStatement();
                // Patch jump at end of then branch to here (end of else)
                poliz[pl_go] = Lex(POLIZ_LABEL, poliz.size());
            } else {
                // If no else, patch false jump to end of if
                poliz[pl_fgo] = Lex(POLIZ_LABEL, poliz.size());
            }
            break;
        }
        case LEX_WHILE: {
            // while (<condition>) <statement>
            gl();
            if (curr_type != LEX_LPAREN) parseError("Expected '(' after 'while'");
            gl();
            int loop_start = poliz.size();
            loop_stack.push_back({loop_start, -1, {}, {}});
            DataType cond_type = parseExpression();
            if (cond_type != D_BOOL) parseError("Condition in 'while' must be boolean");
            if (curr_type != LEX_RPAREN) parseError("Expected ')' after while condition");
            gl();
            // Reserve jump out of loop (if false)
            int pl_fgo = poliz.size();
            emit(Lex());
            emit(Lex(POLIZ_FGO));
            // Set continue target for this loop (start of condition)
            loop_stack.back().continue_target = loop_start;
            // Parse loop body
            parseStatement();
            // End of loop: jump back to start for next iteration
            emit(Lex(POLIZ_LABEL, loop_start));
            emit(Lex(POLIZ_GO));
            // Loop end label
            int loop_end = poliz.size();
            // Patch jump-out on false
            poliz[pl_fgo] = Lex(POLIZ_LABEL, loop_end);
            // Patch any breaks and continues
            for (int pos : loop_stack.back().break_positions) {
                poliz[pos] = Lex(POLIZ_LABEL, loop_end);
            }
            for (int pos : loop_stack.back().continue_positions) {
                poliz[pos] = Lex(POLIZ_LABEL, loop_stack.back().continue_target);
            }
            loop_stack.pop_back();
            break;
        }
        case LEX_DO: {
            // do <statement> while (<condition>) ;
            gl();
            int loop_start = poliz.size();
            loop_stack.push_back({loop_start, -1, {}, {}});
            // Parse loop body
            parseStatement();
            if (curr_type != LEX_WHILE) parseError("Expected 'while' after do-while body");
            gl();
            if (curr_type != LEX_LPAREN) parseError("Expected '(' after 'while'");
            gl();
            // Mark position where condition evaluation begins (for continue)
            int cond_start = poliz.size();
            loop_stack.back().continue_target = cond_start;
            // Patch all continue placeholders to cond_start
            for (int pos : loop_stack.back().continue_positions) {
                poliz[pos] = Lex(POLIZ_LABEL, cond_start);
            }
            DataType cond_type = parseExpression();
            if (cond_type != D_BOOL) parseError("Condition in 'do-while' must be boolean");
            if (curr_type != LEX_RPAREN) parseError("Expected ')' after do-while condition");
            gl();
            // Reserve jump out (if false)
            int pl_fgo = poliz.size();
            emit(Lex());
            emit(Lex(POLIZ_FGO));
            // Jump back to loop start if condition true
            emit(Lex(POLIZ_LABEL, loop_start));
            emit(Lex(POLIZ_GO));
            // End of loop
            int loop_end = poliz.size();
            poliz[pl_fgo] = Lex(POLIZ_LABEL, loop_end);
            // Patch breaks to end
            for (int pos : loop_stack.back().break_positions) {
                poliz[pos] = Lex(POLIZ_LABEL, loop_end);
            }
            loop_stack.pop_back();
            if (curr_type != LEX_SEMICOLON) parseError("Expected ';' after do-while");
            gl();
            break;
        }
        case LEX_FOR: {
            // for ( <init> ; <cond> ; <iter> ) <statement>
            gl();
            if (curr_type != LEX_LPAREN) parseError("Expected '(' after 'for'");
            gl();
            // Parse initialization expression if present
            if (curr_type != LEX_SEMICOLON) {
                parseExpression();
            }
            if (curr_type != LEX_SEMICOLON) parseError("Expected ';' after for-init");
            gl();
            // Mark loop start (condition check position)
            int loop_start = poliz.size();
            loop_stack.push_back({loop_start, -1, {}, {}});
            // Parse loop condition
            int pl_fgo = poliz.size();
            if (curr_type != LEX_SEMICOLON) {
                DataType cond_type = parseExpression();
                if (cond_type != D_BOOL) parseError("Condition in 'for' must be boolean");
            }
            // Reserve jump out of loop (false cond)
            emit(Lex());
            emit(Lex(POLIZ_FGO));
            if (curr_type != LEX_SEMICOLON) parseError("Expected ';' after for-condition");
            gl();
            // Parse iteration expression separately (do not emit to main poliz yet)
            std::vector<Lex> iter_code;
            if (curr_type != LEX_RPAREN) {
                // Temporarily switch output to iter_code vector
                std::vector<Lex>* saved_poliz = cur_poliz;
                cur_poliz = &iter_code;
                parseExpression();
                cur_poliz = saved_poliz;
            }
            if (curr_type != LEX_RPAREN) parseError("Expected ')' after for control");
            gl();
            // Parse loop body
            // (continue will jump to iter_code start, break out to loop end)
            parseStatement();
            // Determine where iteration code will be placed
            int iter_start = poliz.size();
            // Patch continue placeholders to iter_code start
            loop_stack.back().continue_target = iter_start;
            for (int pos : loop_stack.back().continue_positions) {
                poliz[pos] = Lex(POLIZ_LABEL, iter_start);
            }
            // Append iteration code to main poliz
            for (const Lex &lx : iter_code) {
                poliz.push_back(lx);
            }
            // After iteration, jump back to condition check
            emit(Lex(POLIZ_LABEL, loop_start));
            emit(Lex(POLIZ_GO));
            // End of loop
            int loop_end = poliz.size();
            poliz[pl_fgo] = Lex(POLIZ_LABEL, loop_end);
            // Patch breaks to loop_end
            for (int pos : loop_stack.back().break_positions) {
                poliz[pos] = Lex(POLIZ_LABEL, loop_end);
            }
            loop_stack.pop_back();
            break;
        }
        case LEX_BREAK: {
            if (loop_stack.empty()) parseError("Illegal 'break' outside of loop");
            int pl = poliz.size();
            emit(Lex());
            emit(Lex(POLIZ_GO));
            loop_stack.back().break_positions.push_back(pl);
            gl();
            if (curr_type != LEX_SEMICOLON) parseError("Expected ';' after break");
            gl();
            break;
        }
        case LEX_CONTINUE: {
            if (loop_stack.empty()) parseError("Illegal 'continue' outside of loop");
            int pl = poliz.size();
            emit(Lex());
            emit(Lex(POLIZ_GO));
            loop_stack.back().continue_positions.push_back(pl);
            gl();
            if (curr_type != LEX_SEMICOLON) parseError("Expected ';' after continue");
            gl();
            break;
        }
        case LEX_GOTO: {
            gl();
            if (curr_type != LEX_ID) parseError("Expected label identifier after 'goto'");
            std::string labelName = TID[curr_val].name;
            int pl = poliz.size();
            if (label_map.count(labelName)) {
                // Label already defined
                emit(Lex(POLIZ_LABEL, label_map[labelName]));
            } else {
                // Forward reference: emit placeholder and record fixup
                emit(Lex(POLIZ_LABEL, 0));
                label_fixups[labelName].push_back(pl);
            }
            emit(Lex(POLIZ_GO));
            gl();
            if (curr_type != LEX_SEMICOLON) parseError("Expected ';' after goto");
            gl();
            break;
        }
        default:
            if (curr_type == LEX_ID) {
                // Could be a label definition or an assignment
                int id_index = curr_val;
                std::string name = TID[id_index].name;
                gl();
                if (curr_type == LEX_COLON) {
                    // Label definition
                    if (TID[id_index].declare) {
                        parseError("Label name '" + name + "' conflicts with variable");
                    }
                    if (label_map.count(name)) {
                        parseError("Label '" + name + "' already defined");
                    }
                    // Define label at current poliz address
                    label_map[name] = poliz.size();
                    // Patch any pending gotos to this label
                    if (label_fixups.count(name)) {
                        for (int pos : label_fixups[name]) {
                            poliz[pos] = Lex(POLIZ_LABEL, label_map[name]);
                        }
                        label_fixups.erase(name);
                    }
                    gl();  // consume ':'
                    // Continue to parse the statement following the label
                    parseStatement();
                } else {
                    // Not a label: must be an assignment or standalone expression
                    LexType op = curr_type;
                    if (op == LEX_ASSIGN || op == LEX_PLUS_EQ || op == LEX_MINUS_EQ ||
                        op == LEX_TIMES_EQ || op == LEX_SLASH_EQ || op == LEX_PERCENT_EQ) {
                        if (!TID[id_index].declare) {
                            parseError("Undeclared variable '" + name + "' in assignment");
                        }
                        gl();  // consume the operator token
                        // Prepare to generate assignment
                        emit(Lex(POLIZ_ADDRESS, id_index));
                        if (op == LEX_ASSIGN) {
                            DataType expr_type = parseExpression();
                            // Type check
                            LexType varType = TID[id_index].type;
                            if (varType == LEX_BOOL && expr_type != D_BOOL) {
                                parseError("Cannot assign non-bool to bool variable");
                            }
                            if (varType == LEX_STRING && expr_type != D_STRING) {
                                parseError("Cannot assign non-string to string variable");
                            }
                            if ((varType == LEX_INT || varType == LEX_REAL) && !(expr_type == D_INT || expr_type == D_REAL)) {
                                parseError("Cannot assign non-numeric to numeric variable");
                            }
                            emit(Lex(LEX_ASSIGN));
                        } else {
                            // Compound assignment (+=, -=, *=, /=, %=)
                            // Push current value of the variable
                            emit(Lex(LEX_ID, id_index));
                            DataType expr_type = parseExpression();
                            // All compound assignments require numeric operands (except no string support here)
                            if (!(TID[id_index].type == LEX_INT || TID[id_index].type == LEX_REAL) ||
                                !(expr_type == D_INT || expr_type == D_REAL)) {
                                parseError("Operator requires numeric operands");
                            }
                            // Emit the corresponding arithmetic operator
                            LexType opLex;
                            switch (op) {
                                case LEX_PLUS_EQ:   opLex = LEX_PLUS; break;
                                case LEX_MINUS_EQ:  opLex = LEX_MINUS; break;
                                case LEX_TIMES_EQ:  opLex = LEX_TIMES; break;
                                case LEX_SLASH_EQ:  opLex = LEX_SLASH; break;
                                case LEX_PERCENT_EQ:opLex = LEX_PERCENT; break;
                                default: opLex = LEX_PLUS; // should not happen
                            }
                            emit(Lex(opLex));
                            emit(Lex(LEX_ASSIGN));
                        }
                        if (curr_type != LEX_SEMICOLON) {
                            parseError("Expected ';' after assignment");
                        }
                        gl();
                    } else {
                        // If an identifier is not followed by ':' or an assignment operator, it's an error in this language
                        parseError("Unexpected token after identifier '" + name + "'");
                    }
                }
            } else {
                parseError("Unexpected token, unable to parse statement");
            }
    }
}

// <expression> -> <Or>
DataType Parser::parseExpression() {
    return parseOr();
}

// <Or> -> <And> { || <And> }
DataType Parser::parseOr() {
    DataType left_type = parseAnd();
    while (curr_type == LEX_OR) {
        if (left_type != D_BOOL) parseError("Left operand of '||' is not boolean");
        gl();
        DataType right_type = parseAnd();
        if (right_type != D_BOOL) parseError("Right operand of '||' is not boolean");
        emit(Lex(LEX_OR));
        left_type = D_BOOL;
    }
    return left_type;
}

// <And> -> <Equality> { && <Equality> }
DataType Parser::parseAnd() {
    DataType left_type = parseEquality();
    while (curr_type == LEX_AND) {
        if (left_type != D_BOOL) parseError("Left operand of '&&' is not boolean");
        gl();
        DataType right_type = parseEquality();
        if (right_type != D_BOOL) parseError("Right operand of '&&' is not boolean");
        emit(Lex(LEX_AND));
        left_type = D_BOOL;
    }
    return left_type;
}

// <Equality> -> <Relational> { (==|!=) <Relational> }
DataType Parser::parseEquality() {
    DataType left_type = parseRelational();
    while (curr_type == LEX_EQ || curr_type == LEX_NEQ) {
        LexType op = curr_type;
        // Allowed combinations: numeric==numeric, bool==bool, string==string
        if (left_type != D_INT && left_type != D_REAL && left_type != D_BOOL && left_type != D_STRING) {
            parseError("Invalid left operand for equality operator");
        }
        gl();
        DataType right_type = parseRelational();
        bool typeMatch = false;
        if ((left_type == D_INT || left_type == D_REAL) && (right_type == D_INT || right_type == D_REAL)) {
            typeMatch = true;
        } else if (left_type == D_BOOL && right_type == D_BOOL) {
            typeMatch = true;
        } else if (left_type == D_STRING && right_type == D_STRING) {
            typeMatch = true;
        }
        if (!typeMatch) {
            parseError("Operands of '==' or '!=' must be of the same type category");
        }
        emit(Lex(op));
        left_type = D_BOOL;
    }
    return left_type;
}

// <Relational> -> <Additive> { (<|<=|>|>=) <Additive> }
DataType Parser::parseRelational() {
    DataType left_type = parseAdditive();
    while (curr_type == LEX_LESS || curr_type == LEX_GEQ || curr_type == LEX_GREATER || curr_type == LEX_LEQ) {
        LexType op = curr_type;
        if (left_type != D_INT && left_type != D_REAL) {
            parseError("Left operand of relational operator must be numeric");
        }
        gl();
        DataType right_type = parseAdditive();
        if (right_type != D_INT && right_type != D_REAL) {
            parseError("Right operand of relational operator must be numeric");
        }
        emit(Lex(op));
        left_type = D_BOOL;
    }
    return left_type;
}

// <Additive> -> <Term> { (+|-) <Term> }
DataType Parser::parseAdditive() {
    DataType left_type = parseTerm();
    while (curr_type == LEX_PLUS || curr_type == LEX_MINUS) {
        LexType op = curr_type;
        if (op == LEX_PLUS && left_type == D_STRING) {
            // String concatenation
            gl();
            DataType right_type = parseTerm();
            if (right_type != D_STRING) parseError("Cannot concatenate non-string to a string");
            emit(Lex(LEX_PLUS));
            left_type = D_STRING;
            continue;
        }
        // Numeric addition or subtraction
        if (left_type != D_INT && left_type != D_REAL) {
            parseError("Left operand of '+' or '-' is not numeric");
        }
        gl();
        DataType right_type = parseTerm();
        if (right_type != D_INT && right_type != D_REAL) {
            parseError("Right operand of '+' or '-' is not numeric");
        }
        emit(Lex(op));
        // Result type: if either operand is real, result is real, else int
        left_type = (left_type == D_REAL || right_type == D_REAL) ? D_REAL : D_INT;
    }
    return left_type;
}

// <Term> -> <Factor> { (*|/|%) <Factor> }
DataType Parser::parseTerm() {
    DataType left_type = parseFactor();
    while (curr_type == LEX_TIMES || curr_type == LEX_SLASH || curr_type == LEX_PERCENT) {
        LexType op = curr_type;
        if (op == LEX_PERCENT) {
            // Modulus operator (int only)
            if (left_type != D_INT) parseError("Left operand of '%' is not integer");
            gl();
            DataType right_type = parseFactor();
            if (right_type != D_INT) parseError("Right operand of '%' is not integer");
            emit(Lex(LEX_PERCENT));
            left_type = D_INT;
            continue;
        }
        // Multiplication or division
        if (left_type != D_INT && left_type != D_REAL) {
            parseError("Left operand of '*' or '/' is not numeric");
        }
        gl();
        DataType right_type = parseFactor();
        if (right_type != D_INT && right_type != D_REAL) {
            parseError("Right operand of '*' or '/' is not numeric");
        }
        emit(Lex(op));
        left_type = (left_type == D_REAL || right_type == D_REAL) ? D_REAL : D_INT;
    }
    return left_type;
}

// <Factor> -> [ '!' | '-' ] <Factor> | '(' <Expression> ')' | <constant> | <identifier>
DataType Parser::parseFactor() {
    if (curr_type == LEX_NOT) {
        gl();
        DataType t = parseFactor();
        if (t != D_BOOL) parseError("Operand of '!' is not boolean");
        emit(Lex(LEX_NOT));
        return D_BOOL;
    }
    if (curr_type == LEX_MINUS) {
        // Unary minus: treat as 0 - <factor>
        gl();
        int insertPos = poliz.size();
        DataType t = parseFactor();
        if (t != D_INT && t != D_REAL) parseError("Operand of unary '-' is not numeric");
        if (t == D_INT) {
            poliz.insert(poliz.begin() + insertPos, Lex(LEX_INT_CONST, 0));
        } else {
            real_const_table.push_back(0.0);
            int idx = real_const_table.size() - 1;
            poliz.insert(poliz.begin() + insertPos, Lex(LEX_REAL_CONST, idx));
        }
        emit(Lex(LEX_MINUS));
        return t;
    }
    if (curr_type == LEX_LPAREN) {
        gl();
        DataType t = parseExpression();
        if (curr_type != LEX_RPAREN) parseError("Missing ')'");
        gl();
        return t;
    }
    if (curr_type == LEX_ID) {
        int id_index = curr_val;
        if (!TID[id_index].declare) parseError("Use of undeclared identifier: " + TID[id_index].name);
        emit(Lex(LEX_ID, id_index));
        gl();
        // Determine type of the identifier
        LexType var_type = TID[id_index].type;
        if (var_type == LEX_INT)    return D_INT;
        if (var_type == LEX_REAL)   return D_REAL;
        if (var_type == LEX_BOOL)   return D_BOOL;
        if (var_type == LEX_STRING) return D_STRING;
        return D_NONE;
    }
    if (curr_type == LEX_INT_CONST) {
        emit(curr_lex);
        gl();
        return D_INT;
    }
    if (curr_type == LEX_REAL_CONST) {
        emit(curr_lex);
        gl();
        return D_REAL;
    }
    if (curr_type == LEX_TRUE || curr_type == LEX_FALSE) {
        emit(curr_lex);
        gl();
        return D_BOOL;
    }
    if (curr_type == LEX_STR_CONST) {
        emit(curr_lex);
        gl();
        return D_STRING;
    }
    parseError("Unexpected token in expression");
    return D_NONE;  // unreachable
}
