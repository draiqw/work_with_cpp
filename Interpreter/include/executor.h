#ifndef EXECUTER_H
#define EXECUTER_H

#include <vector>
#include "lexer.h"

// Executor class: executes the generated POLIZ code
class Executer {
public:
    void execute(const std::vector<Lex> &poliz);
};

#endif
