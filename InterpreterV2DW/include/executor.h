#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stack>
#include <vector>
#include "lexer.h"

template <class T, class T_EL>
void from_st(T &st, T_EL &i) {
    i = st.top();
    st.pop();
}

class Executer {
public:
    void execute(std::vector<Lex>& poliz);
};

#endif
