#ifndef PARSER_H
#define PARSER_H

#include <stack>
#include <vector>
#include <string>
#include "lexer.h"

class My_struct {
public:
    std::vector<Ident> STID;
    std::string struct_name;
};

class Struct_info {
public:
    std::string st_type;
    std::string st_name;
};

extern std::vector<My_struct> structs;
extern std::vector<Struct_info> info;
int checker(const std::string &name);

class Parser {
    Lex curr_lex;
    type_of_lex c_type;
    int c_val;
    std::string c_str;
    Scanner scan;
    std::stack<type_of_lex> st_lex;
    std::stack<int> ints_stack;
    type_of_lex type;
    type_of_lex type_const;
    int end = 0;
    int goto_address = -1;
    int break_address = -1;
    bool break_flag = false;
    bool goto_flag = false;
    bool lb_flag = false;
    bool cycle_flag = false;

    void P();
    void init_analyze();
    void S();
    void C();
    void H();
    void D();
    void B();
    void O();
    void O1();
    void E();
    void E1();
    void T();
    void F();
    void L();
    void K();
    void J();
    void NN();

    void dec(type_of_lex t);
    void check_init(type_of_lex t, type_of_lex t_c);
    void check_id();
    void check_op(int r, int c);
    void check_not(int r, int c);
    void eq_type(int r, int c);
    void eq_bool(int r, int c);
    void check_id_in_read();
    void gl();
public:
    std::vector<Lex> poliz;
    Parser(const char* program);
    void analyze();
};

#endif
