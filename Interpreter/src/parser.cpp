#include "parser.h"
#include <iostream>
#include <cassert>

// Advance to the next token
void Parser::advance() {
    currentToken = lexer.nextToken();
    // Log token for lexical analysis stage
    std::cout << "[Lexical] Line " << currentToken.line 
              << ": Token `" << currentToken.lexeme << "` (Type " 
              << (int)currentToken.type << ")\n";
}

// Ensure the current token is of the expected type, otherwise report error
void Parser::expect(TokenType expectedType) {
    if (currentToken.type != expectedType) {
        errorFlag = true;
        std::cerr << "Syntax error at line " << currentToken.line 
                  << ": expected token type " << (int)expectedType 
                  << ", got `" << currentToken.lexeme << "`\n";
    }
}

// Add a new variable to symbol table (returns its index in the table)
size_t Parser::addVariable(const std::string& name, ValueType type) {
    // Check for duplicate declaration
    if (varIndexMap.find(name) != varIndexMap.end()) {
        semanticError("Variable `" + name + "` already declared");
        return varIndexMap[name]; // return existing index to continue gracefully
    }
    VarInfo info;
    info.name = name;
    info.type = type;
    info.memIndex = variables.size();
    variables.push_back(info);
    varIndexMap[name] = info.memIndex;
    return info.memIndex;
}

// Get index of a variable by name (or report error if not found)
size_t Parser::getVariableIndex(const std::string& name) {
    if (varIndexMap.find(name) == varIndexMap.end()) {
        semanticError("Use of undeclared variable `" + name + "`");
        // If not found, add it as a dummy variable to avoid cascade errors
        return addVariable(name, ValueType::INT); // default to int
    }
    return varIndexMap[name];
}

// Append an instruction to the intermediate code
void Parser::emit(const Instruction& instr) {
    code.push_back(instr);
}

// Emit a jump instruction with a placeholder target index (to be patched later)
void Parser::emitJump(OpCode op, size_t targetIndexPlaceholder) {
    Instruction instr(op);
    instr.index = targetIndexPlaceholder;
    code.push_back(instr);
}

// Backpatch the target index of a jump instruction at code[instrIndex]
void Parser::backpatch(size_t instrIndex, size_t targetIndex) {
    assert(instrIndex < code.size());
    code[instrIndex].index = targetIndex;
}

// Log semantic error and flag the parser
void Parser::semanticError(const std::string& msg) {
    errorFlag = true;
    std::cerr << "Semantic error at line " << currentToken.line << ": " << msg << std::endl;
}

// Parse the top-level program structure: `program { ... }`
bool Parser::parseProgram() {
    advance();
    expect(TokenType::KW_PROGRAM);
    advance();
    expect(TokenType::LBRACE);
    advance();

    // Parse variable declarations (if any) at the beginning of program
    parseDeclarations();

    // Now parse statements until closing brace
    while (currentToken.type != TokenType::RBRACE && currentToken.type != TokenType::END_OF_FILE) {
        parseStatement();
    }
    expect(TokenType::RBRACE);

    // If parsing ended early due to error, skip to end
    if (currentToken.type != TokenType::END_OF_FILE) {
        // consume remaining tokens if needed
    }

    if (errorFlag) {
        std::cerr << "Parsing failed due to errors.\n";
    } else {
        std::cout << "[Syntactic] Parsing completed successfully.\n";
        // Optionally, output symbol table and intermediate code for debugging:
        std::cout << "[Semantic] Symbol table contains " << variables.size() << " variables.\n";
        std::cout << "[Semantic] Generated " << code.size() << " intermediate code instructions.\n";
    }
    return !errorFlag;
}

// Parse declarations of the form: `<type> <ident> [ = <literal> ] ;` repeated
void Parser::parseDeclarations() {
    // We allow multiple declarations separated by semicolons
    while (currentToken.type == TokenType::KW_INT ||
           currentToken.type == TokenType::KW_REAL ||
           currentToken.type == TokenType::KW_BOOL ||
           currentToken.type == TokenType::KW_STRING) {
        // Determine type
        ValueType vtype;
        switch (currentToken.type) {
            case TokenType::KW_INT:    vtype = ValueType::INT; break;
            case TokenType::KW_REAL:   vtype = ValueType::REAL; break;
            case TokenType::KW_BOOL:   vtype = ValueType::BOOL; break;
            case TokenType::KW_STRING: vtype = ValueType::STRING; break;
            default: vtype = ValueType::INT; break;
        }
        advance(); // consume type keyword

        // One or more variable names, possibly with initialization
        while (true) {
            expect(TokenType::IDENTIFIER);
            std::string varName = currentToken.lexeme;
            // Add variable to symbol table
            size_t varIndex = addVariable(varName, vtype);
            advance();
            // Optional initialization (e.g., int x = 5)
            if (currentToken.type == TokenType::ASSIGN) {
                advance();
                // For simplicity, allow only literal initialization in declarations
                if (currentToken.type == TokenType::INTEGER_LITERAL ||
                    currentToken.type == TokenType::REAL_LITERAL ||
                    currentToken.type == TokenType::STRING_LITERAL ||
                    currentToken.type == TokenType::BOOL_LITERAL) {
                    // Check type compatibility of literal
                    bool typeMatch = true;
                    Instruction pushInstr;
                    if (currentToken.type == TokenType::INTEGER_LITERAL) {
                        if (vtype == ValueType::INT) {
                            pushInstr.op = OpCode::PUSH_INT;
                            pushInstr.intValue = currentToken.intVal;
                        } else if (vtype == ValueType::REAL) {
                            pushInstr.op = OpCode::PUSH_REAL;
                            pushInstr.realValue = (double) currentToken.intVal;
                        } else {
                            typeMatch = false;
                        }
                    } else if (currentToken.type == TokenType::REAL_LITERAL) {
                        if (vtype == ValueType::REAL) {
                            pushInstr.op = OpCode::PUSH_REAL;
                            pushInstr.realValue = currentToken.realVal;
                        } else if (vtype == ValueType::INT) {
                            // real to int initialization (truncate)
                            pushInstr.op = OpCode::PUSH_INT;
                            pushInstr.intValue = (int) currentToken.realVal;
                        } else {
                            typeMatch = false;
                        }
                    } else if (currentToken.type == TokenType::BOOL_LITERAL) {
                        if (vtype == ValueType::BOOL) {
                            pushInstr.op = OpCode::PUSH_BOOL;
                            pushInstr.boolValue = currentToken.boolVal;
                        } else {
                            typeMatch = false;
                        }
                    } else if (currentToken.type == TokenType::STRING_LITERAL) {
                        if (vtype == ValueType::STRING) {
                            pushInstr.op = OpCode::PUSH_STRING;
                            pushInstr.stringValue = currentToken.lexeme;
                        } else {
                            typeMatch = false;
                        }
                    }
                    if (!typeMatch) {
                        semanticError("Type mismatch in initialization of `" + varName + "`");
                    } else {
                        // Emit code to initialize the variable at runtime
                        emit(pushInstr);
                        // Emit address of variable and assignment
                        Instruction addr;
                        addr.op = OpCode::PUSH_ADDR;
                        addr.index = varIndex;
                        emit(addr);
                        emit(Instruction(OpCode::ASSIGN));
                    }
                } else {
                    semanticError("Expected a literal value for initialization of `" + varName + "`");
                }
                advance();
            }
            if (currentToken.type == TokenType::COMMA) {
                advance();
                continue; // another variable in the same declaration
            } else {
                break;
            }
        }
        expect(TokenType::SEMICOLON);
        advance();
    }
}

// Parse a single statement (or a sequence in a compound block)
void Parser::parseStatement() {
    switch (currentToken.type) {
        case TokenType::LBRACE:
            parseCompoundStatement();
            break;
        case TokenType::KW_IF:
            parseIfStatement();
            break;
        case TokenType::KW_WHILE:
            parseWhileStatement();
            break;
        case TokenType::KW_DO:
            parseDoWhileStatement();
            break;
        case TokenType::KW_FOR:
            parseForStatement();
            break;
        case TokenType::KW_CASE:
            parseCaseStatement();
            break;
        case TokenType::KW_BREAK:
            parseBreakStatement();
            break;
        case TokenType::KW_CONTINUE:
            parseContinueStatement();
            break;
        case TokenType::KW_GOTO:
            parseGotoStatement();
            break;
        case TokenType::KW_READ:
            parseReadStatement();
            break;
        case TokenType::KW_WRITE:
            parseWriteStatement();
            break;
        case TokenType::SEMICOLON:
            // Empty statement
            advance();
            break;
        default:
            // Assume it's an expression statement or label definition
            if (currentToken.type == TokenType::IDENTIFIER) {
                // Look ahead to see if it's a label (identifier followed by colon)
                Token nextTok = lexer.peekToken();
                if (nextTok.type == TokenType::COLON) {
                    // Label definition
                    std::string labelName = currentToken.lexeme;
                    // Define label at current code index
                    if (labelMap.find(labelName) == labelMap.end()) {
                        labelMap[labelName] = { code.size(), true, {} };
                    } else {
                        if (labelMap[labelName].defined) {
                            semanticError("Duplicate label `" + labelName + "`");
                        }
                        // Patch any gotos waiting for this label
                        labelMap[labelName].defined = true;
                        for (size_t instrIdx : labelMap[labelName].patchLocations) {
                            backpatch(instrIdx, code.size());
                        }
                        labelMap[labelName].patchLocations.clear();
                        labelMap[labelName].index = code.size();
                    }
                    advance(); // consume label identifier
                    advance(); // consume colon
                    // After a label, expect a statement
                    parseStatement();
                    return;
                }
            }
            // Otherwise, parse as expression statement
            {
                ValueType exprType = parseExpression();
                // Pop the result of expression if not used (to clean stack)
                if (exprType != ValueType::VOID) {
                    // Insert a pop (no-op) if expression produces a value to discard
                    // In our bytecode, we could push a NOP or simply ignore result.
                    // We'll ignore by not generating any specific pop instruction here for simplicity.
                }
            }
            expect(TokenType::SEMICOLON);
            advance();
            break;
    }
}

// Parse a block: '{' <statements> '}'
void Parser::parseCompoundStatement() {
    expect(TokenType::LBRACE);
    advance();
    while (currentToken.type != TokenType::RBRACE && currentToken.type != TokenType::END_OF_FILE) {
        parseStatement();
    }
    expect(TokenType::RBRACE);
    advance();
}

// Parse an if-else statement: 'if (<expr>) <stmt> [else <stmt>]'
void Parser::parseIfStatement() {
    expect(TokenType::KW_IF);
    advance();
    expect(TokenType::LPAREN);
    advance();
    // Parse condition expression
    ValueType condType = parseExpression();
    if (condType != ValueType::BOOL) {
        semanticError("Non-boolean expression in if condition");
    }
    expect(TokenType::RPAREN);
    advance();
    // Prepare jump for false condition
    // Reserve an instruction for "if false -> jump to else/end"
    size_t jmpFalseIndex = code.size();
    emitJump(OpCode::JMP_IF_FALSE, 0);  // placeholder 0 for now
    // Parse true branch
    parseStatement();
    // Now either an else or not
    if (currentToken.type == TokenType::KW_ELSE) {
        // If else present, prepare an unconditional jump to skip the else part after executing true branch
        size_t jmpEndIndex = code.size();
        emitJump(OpCode::JMP, 0);
        // Patch the false jump to point here (beginning of else block)
        backpatch(jmpFalseIndex, code.size());
        advance(); // consume 'else'
        parseStatement();
        // Patch the end-of-if jump to here (after else branch)
        backpatch(jmpEndIndex, code.size());
    } else {
        // No else: patch false jump to this point (end of if)
        backpatch(jmpFalseIndex, code.size());
    }
}

// Parse a while loop: 'while (<expr>) <stmt>'
void Parser::parseWhileStatement() {
    expect(TokenType::KW_WHILE);
    advance();
    expect(TokenType::LPAREN);
    advance();
    // Mark loop start label
    size_t loopStartIndex = code.size();
    // Parse loop condition
    ValueType condType = parseExpression();
    if (condType != ValueType::BOOL) {
        semanticError("Non-boolean expression in while condition");
    }
    expect(TokenType::RPAREN);
    advance();
    // Emit jump if false to loop end (placeholder)
    size_t jmpFalseIndex = code.size();
    emitJump(OpCode::JMP_IF_FALSE, 0);
    // Push loop context for break/continue
    std::string breakLabel = "L_end_loop_" + std::to_string(loopStack.size());
    std::string continueLabel = "L_start_loop_" + std::to_string(loopStack.size());
    loopStack.push_back({ breakLabel, continueLabel });
    parseStatement();
    // After loop body, emit jump back to loop start
    emitJump(OpCode::JMP, loopStartIndex);
    // Mark loop end position and patch the false jump
    size_t loopEndIndex = code.size();
    backpatch(jmpFalseIndex, loopEndIndex);
    // Patch any break/continue placeholders for this loop
    // (Break jumps patched to loopEndIndex, continue jumps patched to loopStartIndex)
    if (labelMap.find(breakLabel) != labelMap.end()) {
        for (size_t instrIdx : labelMap[breakLabel].patchLocations) {
            backpatch(instrIdx, loopEndIndex);
        }
    }
    if (labelMap.find(continueLabel) != labelMap.end()) {
        for (size_t instrIdx : labelMap[continueLabel].patchLocations) {
            backpatch(instrIdx, loopStartIndex);
        }
    }
    // Pop loop context
    loopStack.pop_back();
}

// Parse a do-while loop: 'do <stmt> while (<expr>);'
void Parser::parseDoWhileStatement() {
    expect(TokenType::KW_DO);
    advance();
    // Mark loop start
    size_t loopStartIndex = code.size();
    // Set up loop context labels for break/continue
    std::string breakLabel = "L_end_loop_" + std::to_string(loopStack.size());
    std::string continueLabel = "L_check_loop_" + std::to_string(loopStack.size()); // continue jumps to condition check
    loopStack.push_back({ breakLabel, continueLabel });
    // Parse loop body
    parseStatement();
    expect(TokenType::KW_WHILE);
    advance();
    expect(TokenType::LPAREN);
    advance();
    // Mark point for continue (condition evaluation)
    size_t condCheckIndex = code.size();
    // If any continue statements in loop, patch them to condition check
    if (labelMap.find(continueLabel) != labelMap.end()) {
        for (size_t instrIdx : labelMap[continueLabel].patchLocations) {
            backpatch(instrIdx, condCheckIndex);
        }
    }
    // Parse loop condition
    ValueType condType = parseExpression();
    if (condType != ValueType::BOOL) {
        semanticError("Non-boolean expression in do-while condition");
    }
    expect(TokenType::RPAREN);
    advance();
    expect(TokenType::SEMICOLON);
    advance();
    // Emit jump back to loop start if condition true
    emitJump(OpCode::JMP_IF_FALSE, code.size() + 2); // if false, skip the next jump (which jumps to start)
    emitJump(OpCode::JMP, loopStartIndex);
    // Here code.size() + 2 is used assuming each emitJump adds one instruction. Actually, simpler:
    // We can invert logic: emit if false jump out and unconditional jump to start after.
    // (For clarity, an alternate implementation could use a single conditional jump).
    // In our approach: The sequence above emits two instructions:
    // JMP_IF_FALSE (with target set to after the next JMP)
    // JMP (to loopStartIndex).
    // Execution: if condition false, it skips the JMP back (exits loop); if true, it runs the JMP to start.
    // Mark loop end after these jumps:
    size_t loopEndIndex = code.size();
    // Patch any break placeholders to loopEndIndex
    if (labelMap.find(breakLabel) != labelMap.end()) {
        for (size_t instrIdx : labelMap[breakLabel].patchLocations) {
            backpatch(instrIdx, loopEndIndex);
        }
    }
    // Pop loop context
    loopStack.pop_back();
}

// Parse a for loop: 'for (<init>; <cond>; <update>) <stmt>'
void Parser::parseForStatement() {
    expect(TokenType::KW_FOR);
    advance();
    expect(TokenType::LPAREN);
    advance();
    // Parse initialization expression (or skip if empty)
    if (currentToken.type != TokenType::SEMICOLON) {
        parseExpression();
    }
    expect(TokenType::SEMICOLON);
    advance();
    // Mark condition check position
    size_t condCheckIndex = code.size();
    // Parse loop condition
    size_t jmpFalseIndex = 0;
    if (currentToken.type != TokenType::SEMICOLON) {
        ValueType condType = parseExpression();
        if (condType != ValueType::BOOL) {
            semanticError("Non-boolean expression in for-loop condition");
        }
        // Emit jump out of loop if condition false
        jmpFalseIndex = code.size();
        emitJump(OpCode::JMP_IF_FALSE, 0);
    }
    expect(TokenType::SEMICOLON);
    advance();
    // Record location of update expression (to jump to after each iteration)
    size_t updateIndex = code.size();
    // We will emit jump to end of loop body to skip update on first iteration
    // Actually, easier: skip for now, parse update after parsing body via a separate logic
    // We'll parse update as expression but not emit it immediately, or store it to emit at correct place.
    // Simpler: we can rearrange typical for loop into while form.
    // We'll adopt structure:
    // init;
    // loopStart:
    // if(cond false) goto end;
    // body;
    // continueLabel:
    // update;
    // goto loopStart;
    // endLabel:
    //
    // To implement, we might parse update as expression and hold its instructions aside.
    std::vector<Instruction> updateCode;
    if (currentToken.type != TokenType::RPAREN) {
        // Parse update expression(s) (we allow comma-separated as one expression)
        // We'll capture its code by noting current code size and generating it, then extracting it.
        size_t updateStart = code.size();
        ValueType updType = parseExpression();
        (void)updType; // result not used
        // The update code is from updateStart to current code end
        for (size_t i = updateStart; i < code.size(); ++i) {
            updateCode.push_back(code[i]);
        }
        // Remove it from main code for now (we will insert at continue point)
        code.resize(updateStart);
    }
    expect(TokenType::RPAREN);
    advance();
    // Set up loop context labels
    std::string breakLabel = "L_end_loop_" + std::to_string(loopStack.size());
    std::string continueLabel = "L_update_loop_" + std::to_string(loopStack.size());
    loopStack.push_back({ breakLabel, continueLabel });
    // Mark loop start label for continue (after condition check)
    size_t loopStartIndex = code.size();
    // Parse loop body
    parseStatement();
    // Upon reaching end of body, emit update code and jump back to condition
    size_t updateStartIndex = code.size();
    // Patch any 'continue' statements in loop to here (start of update section)
    if (labelMap.find(continueLabel) != labelMap.end()) {
        for (size_t instrIdx : labelMap[continueLabel].patchLocations) {
            backpatch(instrIdx, updateStartIndex);
        }
    }
    // Append the stored update instructions
    for (auto& instr : updateCode) {
        code.push_back(instr);
    }
    // After update, jump back to condition check
    emitJump(OpCode::JMP, condCheckIndex);
    // Determine loop end index
    size_t loopEndIndex = code.size();
    // Patch condition false jump to loopEndIndex
    if (jmpFalseIndex != 0) {
        backpatch(jmpFalseIndex, loopEndIndex);
    }
    // Patch break statements to loopEndIndex
    if (labelMap.find(breakLabel) != labelMap.end()) {
        for (size_t instrIdx : labelMap[breakLabel].patchLocations) {
            backpatch(instrIdx, loopEndIndex);
        }
    }
    loopStack.pop_back();
}

// Parse a switch-case (variant: 'case' statement in this custom language).
// We'll assume syntax: 'case (<expr>) { case <const>: ... [ break; ] ... default: ... }'
void Parser::parseCaseStatement() {
    expect(TokenType::KW_CASE);
    advance();
    expect(TokenType::LPAREN);
    advance();
    // Parse expression to switch on
    ValueType exprType = parseExpression();
    // For simplicity, we restrict switch expression to int or bool
    if (exprType != ValueType::INT && exprType != ValueType::BOOL && exprType != ValueType::STRING) {
        semanticError("Switch expression must be int, bool, or string type");
    }
    expect(TokenType::RPAREN);
    advance();
    expect(TokenType::LBRACE);
    advance();
    // Setup switch context for break handling
    std::string breakLabel = "L_end_switch_" + std::to_string(switchStack.size());
    switchStack.push_back({ breakLabel });
    // Save the location of the switch expression value by storing it in a temp variable
    // We'll allocate a new temp variable for the expression (not directly accessible by user)
    std::string tempName = "#switch" + std::to_string(code.size());
    ValueType tempType = exprType;
    size_t tempIndex = addVariable(tempName, tempType);
    // Emit code to assign expression result to temp variable
    Instruction addr;
    addr.op = OpCode::PUSH_ADDR;
    addr.index = tempIndex;
    emit(addr);
    // The expression result is already on stack, so just assign into temp
    emit(Instruction(OpCode::ASSIGN));
    // Prepare to collect case jump instructions for chain
    std::vector<size_t> jumpChainIndices;
    std::vector<int> caseIntValues;
    std::vector<std::string> caseStrValues;
    size_t defaultJumpIndex = SIZE_MAX;
    bool defaultDefined = false;
    // Parse case labels inside the block
    while (currentToken.type == TokenType::KW_CASE || currentToken.type == TokenType::KW_DEFAULT) {
        if (currentToken.type == TokenType::KW_CASE) {
            // case constant
            advance();
            // We expect a literal constant for case label
            if (currentToken.type == TokenType::INTEGER_LITERAL || 
                currentToken.type == TokenType::BOOL_LITERAL ||
                currentToken.type == TokenType::STRING_LITERAL) {
                // Record the case value for jump generation
                if (currentToken.type == TokenType::STRING_LITERAL) {
                    caseStrValues.push_back(currentToken.lexeme);
                } else {
                    // Booleans are represented as boolVal or intVal (true=1, false=0)
                    int val = 0;
                    if (currentToken.type == TokenType::BOOL_LITERAL) {
                        val = currentToken.boolVal ? 1 : 0;
                    } else {
                        val = currentToken.intVal;
                    }
                    caseIntValues.push_back(val);
                }
            } else {
                semanticError("Case label must be a constant literal");
            }
            advance();
            expect(TokenType::COLON);
            advance();
            // At this point, before generating statements for this case, generate the comparison and jump logic:
            // Compare temp with case value -> if equal, jump into case body, else skip.
            // We will emit this after collecting all branch positions (for simplicity, assume default at end).
            // Actually, for simplicity in code generation, we'll generate chain now:
            // Emit code: PUSH_VAR temp, PUSH_CONST caseVal, EQ, JMP_IF_FALSE (to next label, placeholder)
            Instruction loadTemp;
            loadTemp.op = OpCode::PUSH_VAR;
            loadTemp.index = tempIndex;
            emit(loadTemp);
            // Emit push constant for case value
            if (!caseStrValues.empty()) {
                // last added was a string
                Instruction pushC;
                pushC.op = OpCode::PUSH_STRING;
                pushC.stringValue = caseStrValues.back();
                emit(pushC);
            } else {
                // last added was int/bool
                Instruction pushC;
                if (exprType == ValueType::REAL) {
                    // (Not likely real in switch, but handle generically)
                    pushC.op = OpCode::PUSH_REAL;
                    pushC.realValue = (double) caseIntValues.back();
                } else {
                    pushC.op = OpCode::PUSH_INT;
                    pushC.intValue = caseIntValues.back();
                }
                emit(pushC);
            }
            // Emit equality comparison
            emit(Instruction(OpCode::EQ));
            // Reserve jump if false (skip this case) with placeholder
            size_t jfIndex = code.size();
            emitJump(OpCode::JMP_IF_FALSE, 0);
            jumpChainIndices.push_back(jfIndex);
            // Now, case body code will follow
            // Parse statements for this case (until break or next label/default)
            while (currentToken.type != TokenType::KW_CASE && 
                   currentToken.type != TokenType::KW_DEFAULT &&
                   currentToken.type != TokenType::RBRACE && 
                   currentToken.type != TokenType::END_OF_FILE) {
                parseStatement();
            }
            // If no break at end of case, execution will fall through to next case (which is naturally the next code).
            // If a break occurred, it emitted a jump to end label via parseBreakStatement.
            // We'll handle break patching later with switchStack.
        } else if (currentToken.type == TokenType::KW_DEFAULT) {
            // default label
            advance();
            expect(TokenType::COLON);
            advance();
            defaultDefined = true;
            // If we encountered default, patch all previous case JMP_IF_FALSE to jump here (default code)
            for (size_t idx : jumpChainIndices) {
                backpatch(idx, code.size());
            }
            jumpChainIndices.clear();
            // Parse default case statements
            while (currentToken.type != TokenType::KW_CASE &&
                   currentToken.type != TokenType::RBRACE && 
                   currentToken.type != TokenType::END_OF_FILE) {
                parseStatement();
            }
        }
    }
    expect(TokenType::RBRACE);
    advance();
    // End of switch: patch any remaining chain jumps (cases without default)
    for (size_t idx : jumpChainIndices) {
        backpatch(idx, code.size());
    }
    // Patch break statements in this switch to jump here (end of switch)
    if (labelMap.find(breakLabel) != labelMap.end()) {
        for (size_t instrIdx : labelMap[breakLabel].patchLocations) {
            backpatch(instrIdx, code.size());
        }
    }
    switchStack.pop_back();
}

// Parse a goto statement: 'goto <label>;'
void Parser::parseGotoStatement() {
    expect(TokenType::KW_GOTO);
    advance();
    expect(TokenType::IDENTIFIER);
    std::string labelName = currentToken.lexeme;
    // Emit an unconditional jump instruction, but we need to patch its target later.
    size_t jmpIndex = code.size();
    emitJump(OpCode::JMP, 0);
    // Record this jump in labelMap for patching once label is defined
    if (labelMap.find(labelName) == labelMap.end()) {
        labelMap[labelName] = { 0, false, { jmpIndex } };
    } else {
        labelMap[labelName].patchLocations.push_back(jmpIndex);
    }
    advance();
    expect(TokenType::SEMICOLON);
    advance();
}

// Parse a read statement: 'read(<ident>);'
void Parser::parseReadStatement() {
    expect(TokenType::KW_READ);
    advance();
    expect(TokenType::LPAREN);
    advance();
    expect(TokenType::IDENTIFIER);
    std::string varName = currentToken.lexeme;
    size_t varIndex = getVariableIndex(varName);
    // Emit address of variable and then READ instruction
    Instruction addr;
    addr.op = OpCode::PUSH_ADDR;
    addr.index = varIndex;
    emit(addr);
    emit(Instruction(OpCode::READ));
    advance();
    expect(TokenType::RPAREN);
    advance();
    expect(TokenType::SEMICOLON);
    advance();
}

// Parse a write statement: 'write(<expr_list>);'
// Supports multiple comma-separated expressions inside write
void Parser::parseWriteStatement() {
    expect(TokenType::KW_WRITE);
    advance();
    expect(TokenType::LPAREN);
    advance();
    // We allow multiple expressions separated by comma
    bool first = true;
    while (currentToken.type != TokenType::RPAREN && currentToken.type != TokenType::END_OF_FILE) {
        ValueType exprType = parseExpression();
        // Emit a WRITE instruction for each expression
        emit(Instruction(OpCode::WRITE));
        if (currentToken.type == TokenType::COMMA) {
            advance();
            // continue parsing next expression
        } else {
            break;
        }
    }
    expect(TokenType::RPAREN);
    advance();
    expect(TokenType::SEMICOLON);
    advance();
}

// Parse a break statement
void Parser::parseBreakStatement() {
    expect(TokenType::KW_BREAK);
    // Find the nearest loop or switch context to break out of
    if (!loopStack.empty() || !switchStack.empty()) {
        // Determine break target label:
        std::string label;
        if (!loopStack.empty()) {
            label = loopStack.back().breakLabel;
        } else {
            label = switchStack.back().breakLabel;
        }
        // Emit jump to break label (we'll patch it to end of loop/switch later)
        size_t jmpIndex = code.size();
        emitJump(OpCode::JMP, 0);
        // Record for patching
        if (labelMap.find(label) == labelMap.end()) {
            labelMap[label] = {0, false, { jmpIndex } };
        } else {
            labelMap[label].patchLocations.push_back(jmpIndex);
        }
    } else {
        semanticError("`break` used outside of loop or switch");
    }
    advance();
    expect(TokenType::SEMICOLON);
    advance();
}

// Parse a continue statement
void Parser::parseContinueStatement() {
    expect(TokenType::KW_CONTINUE);
    if (!loopStack.empty()) {
        std::string contLabel = loopStack.back().continueLabel;
        size_t jmpIndex = code.size();
        emitJump(OpCode::JMP, 0);
        if (labelMap.find(contLabel) == labelMap.end()) {
            labelMap[contLabel] = {0, false, { jmpIndex } };
        } else {
            labelMap[contLabel].patchLocations.push_back(jmpIndex);
        }
    } else {
        semanticError("`continue` used outside of loop");
    }
    advance();
    expect(TokenType::SEMICOLON);
    advance();
}

// ---- Expression parsing methods (precedence climbing recursive descent) ----

// parseExpression: parse assignment expressions (with '=' if present)
ValueType Parser::parseExpression() {
    // We allow assignment operator '=' to assign to a variable
    // Check if current token is an identifier that could be assigned to
    if (currentToken.type == TokenType::IDENTIFIER) {
        // Look ahead for '='
        Token tok = lexer.peekToken();
        if (tok.type == TokenType::ASSIGN) {
            // Assignment detected
            std::string varName = currentToken.lexeme;
            size_t varIndex = getVariableIndex(varName);
            ValueType varType = variables[varIndex].type;
            advance(); // consume identifier
            expect(TokenType::ASSIGN);
            advance();
            // Emit an address-of-variable for assignment target
            Instruction addr;
            addr.op = OpCode::PUSH_ADDR;
            addr.index = varIndex;
            emit(addr);
            // Parse right-hand side expression
            ValueType rhsType = parseExpression();
            // Type check: allow numeric conversion int<->real
            if ((varType == ValueType::INT && rhsType == ValueType::REAL) ||
                (varType == ValueType::REAL && rhsType == ValueType::INT)) {
                // emit conversion if needed (int->real or real->int)
                // For simplicity, if assigning real to int, we'll truncate (as in C semantics)
            } else if (varType != rhsType && rhsType != ValueType::VOID) {
                semanticError("Type mismatch in assignment to `" + varName + "`");
            }
            // Emit assignment operation
            emit(Instruction(OpCode::ASSIGN));
            // Assignment expression result is the value assigned (we will leave it on stack as value)
            return varType;
        }
    }
    // If not an assignment, parse a logical OR expression (the next level down)
    return parseLogicalOrExpression();
}

// parseLogicalOrExpression: E || E
ValueType Parser::parseLogicalOrExpression() {
    ValueType leftType = parseLogicalAndExpression();
    while (currentToken.type == TokenType::OP_OR) {
        // Short-circuit semantics: if left is false then evaluate right
        if (leftType != ValueType::BOOL) {
            semanticError("Operand of '||' is not boolean");
        }
        // Reserve jump-if-true to skip evaluating right-hand side if left is true
        size_t skipRight = code.size();
        emitJump(OpCode::JMP, 0); // Actually, in lazy eval we'd do something like JMP_IF_FALSE to evaluate second.
        // But for simplicity, we'll fully evaluate both sides and then OR them (no short-circuit implemented here).
        advance();
        ValueType rightType = parseLogicalAndExpression();
        if (rightType != ValueType::BOOL) {
            semanticError("Operand of '||' is not boolean");
        }
        // Emit OR operation
        emit(Instruction(OpCode::OR));
        leftType = ValueType::BOOL;
        // (Short-circuit not fully implemented in this simple code generation)
        // backpatch skipRight if needed...
    }
    return leftType;
}

// parseLogicalAndExpression: E && E
ValueType Parser::parseLogicalAndExpression() {
    ValueType leftType = parseEqualityExpression();
    while (currentToken.type == TokenType::OP_AND) {
        if (leftType != ValueType::BOOL) {
            semanticError("Operand of '&&' is not boolean");
        }
        advance();
        ValueType rightType = parseEqualityExpression();
        if (rightType != ValueType::BOOL) {
            semanticError("Operand of '&&' is not boolean");
        }
        emit(Instruction(OpCode::AND));
        leftType = ValueType::BOOL;
    }
    return leftType;
}

// parseEqualityExpression: E == E or E != E
ValueType Parser::parseEqualityExpression() {
    ValueType leftType = parseRelationalExpression();
    while (currentToken.type == TokenType::OP_EQ || currentToken.type == TokenType::OP_NE) {
        TokenType opType = currentToken.type;
        advance();
        ValueType rightType = parseRelationalExpression();
        // Type checking: allow comparison of same types, or int vs real
        if ((leftType == ValueType::INT || leftType == ValueType::REAL) &&
            (rightType == ValueType::INT || rightType == ValueType::REAL)) {
            // numeric comparisons are fine, promote to real if needed
        } else if (leftType != rightType) {
            semanticError("Type mismatch in equality comparison");
        }
        // Emit appropriate comparison operation
        Instruction instr;
        instr.op = (opType == TokenType::OP_EQ ? OpCode::EQ : OpCode::NE);
        emit(instr);
        leftType = ValueType::BOOL;
    }
    return leftType;
}

// parseRelationalExpression: E < E, E <= E, E > E, E >= E
ValueType Parser::parseRelationalExpression() {
    ValueType leftType = parseAdditiveExpression();
    while (currentToken.type == TokenType::OP_LT || currentToken.type == TokenType::OP_LE ||
           currentToken.type == TokenType::OP_GT || currentToken.type == TokenType::OP_GE) {
        TokenType opType = currentToken.type;
        advance();
        ValueType rightType = parseAdditiveExpression();
        // Type checking similar to equality: numeric types can be compared, strings with strings, etc.
        if ((leftType == ValueType::INT || leftType == ValueType::REAL) &&
            (rightType == ValueType::INT || rightType == ValueType::REAL)) {
            // ok
        } else if (leftType != rightType) {
            semanticError("Type mismatch in relational comparison");
        }
        // Emit comparison operation
        Instruction instr;
        switch (opType) {
            case TokenType::OP_LT: instr.op = OpCode::LT; break;
            case TokenType::OP_LE: instr.op = OpCode::LE; break;
            case TokenType::OP_GT: instr.op = OpCode::GT; break;
            case TokenType::OP_GE: instr.op = OpCode::GE; break;
            default: instr.op = OpCode::NOP; break;
        }
        emit(instr);
        leftType = ValueType::BOOL;
    }
    return leftType;
}

// parseAdditiveExpression: E + E or E - E
ValueType Parser::parseAdditiveExpression() {
    ValueType leftType = parseMultiplicativeExpression();
    while (currentToken.type == TokenType::OP_PLUS || currentToken.type == TokenType::OP_MINUS) {
        TokenType opType = currentToken.type;
        advance();
        ValueType rightType = parseMultiplicativeExpression();
        // Numeric or string concatenation
        if (leftType == ValueType::STRING || rightType == ValueType::STRING) {
            if (opType == TokenType::OP_PLUS && leftType == ValueType::STRING && rightType == ValueType::STRING) {
                // string concatenation
            } else {
                semanticError("Invalid types for '+' operation");
            }
        }
        // If numeric, allow int/real promotion
        if ((leftType == ValueType::INT || leftType == ValueType::REAL) &&
            (rightType == ValueType::INT || rightType == ValueType::REAL)) {
            // result numeric type (real if either is real)
            if (leftType == ValueType::REAL || rightType == ValueType::REAL) {
                leftType = ValueType::REAL;
            } else {
                leftType = ValueType::INT;
            }
        }
        // Emit add or subtract
        Instruction instr;
        instr.op = (opType == TokenType::OP_PLUS ? OpCode::ADD : OpCode::SUB);
        emit(instr);
    }
    return leftType;
}

// parseMultiplicativeExpression: E * E, E / E, E % E
ValueType Parser::parseMultiplicativeExpression() {
    ValueType leftType = parseUnaryExpression();
    while (currentToken.type == TokenType::OP_MUL || currentToken.type == TokenType::OP_DIV || currentToken.type == TokenType::OP_MOD) {
        TokenType opType = currentToken.type;
        advance();
        ValueType rightType = parseUnaryExpression();
        if ((leftType == ValueType::INT || leftType == ValueType::REAL) &&
            (rightType == ValueType::INT || rightType == ValueType::REAL)) {
            if (leftType == ValueType::REAL || rightType == ValueType::REAL) {
                leftType = ValueType::REAL;
            } else {
                leftType = ValueType::INT;
            }
            // If op is '%' and operands are not int, error
            if (opType == TokenType::OP_MOD && (leftType != ValueType::INT || rightType != ValueType::INT)) {
                semanticError("Modulo operator requires integer operands");
            }
        } else {
            semanticError("Invalid operand types for binary operation");
        }
        // Emit corresponding operation
        Instruction instr;
        if (opType == TokenType::OP_MUL) instr.op = OpCode::MUL;
        if (opType == TokenType::OP_DIV) instr.op = OpCode::DIV;
        if (opType == TokenType::OP_MOD) instr.op = OpCode::MOD;
        emit(instr);
    }
    return leftType;
}

// parseUnaryExpression: [ '!' | '-' ] PrimaryExpression
ValueType Parser::parseUnaryExpression() {
    if (currentToken.type == TokenType::OP_NOT) {
        advance();
        ValueType type = parseUnaryExpression();
        if (type != ValueType::BOOL) {
            semanticError("Operand of '!' must be boolean");
        }
        emit(Instruction(OpCode::NOT));
        return ValueType::BOOL;
    } else if (currentToken.type == TokenType::OP_MINUS) {
        // unary minus
        advance();
        ValueType type = parseUnaryExpression();
        if (type == ValueType::INT || type == ValueType::REAL) {
            // Emit a 0 value then SUB if needed to negate, or have a NEG opcode.
            // We'll do: 0 - value
            Instruction zero;
            if (type == ValueType::INT) {
                zero.op = OpCode::PUSH_INT;
                zero.intValue = 0;
            } else {
                zero.op = OpCode::PUSH_REAL;
                zero.realValue = 0.0;
            }
            emit(zero);
            emit(Instruction(OpCode::SUB));
        } else {
            semanticError("Unary '-' on non-numeric type");
        }
        return type;
    }
    return parsePrimaryExpression();
}

// parsePrimaryExpression: literals, identifiers, or parenthesized expression
ValueType Parser::parsePrimaryExpression() {
    ValueType type = ValueType::VOID;
    if (currentToken.type == TokenType::IDENTIFIER) {
        // Variable usage
        std::string name = currentToken.lexeme;
        size_t idx = getVariableIndex(name);
        type = variables[idx].type;
        // Emit push variable value
        Instruction instr;
        instr.op = OpCode::PUSH_VAR;
        instr.index = idx;
        emit(instr);
        advance();
    } else if (currentToken.type == TokenType::INTEGER_LITERAL) {
        type = ValueType::INT;
        Instruction instr;
        instr.op = OpCode::PUSH_INT;
        instr.intValue = currentToken.intVal;
        emit(instr);
        advance();
    } else if (currentToken.type == TokenType::REAL_LITERAL) {
        type = ValueType::REAL;
        Instruction instr;
        instr.op = OpCode::PUSH_REAL;
        instr.realValue = currentToken.realVal;
        emit(instr);
        advance();
    } else if (currentToken.type == TokenType::BOOL_LITERAL) {
        type = ValueType::BOOL;
        Instruction instr;
        instr.op = OpCode::PUSH_BOOL;
        instr.boolValue = currentToken.boolVal;
        emit(instr);
        advance();
    } else if (currentToken.type == TokenType::STRING_LITERAL) {
        type = ValueType::STRING;
        Instruction instr;
        instr.op = OpCode::PUSH_STRING;
        instr.stringValue = currentToken.lexeme;
        emit(instr);
        advance();
    } else if (currentToken.type == TokenType::LPAREN) {
        advance();
        type = parseExpression();
        expect(TokenType::RPAREN);
        advance();
    } else {
        semanticError("Unexpected token in expression: " + currentToken.lexeme);
        advance();
    }
    return type;
}
