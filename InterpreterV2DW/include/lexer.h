#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <string>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <vector>
#include <stack>
#include <algorithm>

#define BUFSIZE 256

enum type_of_lex {
    LEX_NULL,                                /*0*/
    LEX_AND, LEX_BOOL, LEX_DO, LEX_ELSE, LEX_IF, LEX_FALSE, LEX_INT, LEX_NOT,
    LEX_OR, LEX_PROGRAM,                     /*10*/
    LEX_READ, LEX_STRING, LEX_TRUE, LEX_GOTO, LEX_BREAK, LEX_WHILE, LEX_WRITE, LEX_FOR, /*18*/
    TYPE_STRUCT,                             /*19*/
    LEX_QUOTE,                               /*20*/
    LEX_BEGIN, LEX_END, LEX_SEMICOLON, LEX_COMMA, LEX_COLON, LEX_NEQ,
    LEX_ASSIGN, LEX_LPAREN,                 /*28*/
    LEX_RPAREN, LEX_EQ, LEX_LSS, LEX_GTR, LEX_PLUS, LEX_MINUS, LEX_TIMES,
    LEX_SLASH, LEX_LEQ, LEX_GEQ,            /*38*/
    LEX_PERCENT,                            /*39*/
    LEX_NUM,                                /*40*/
    LEX_STR,                                /*41*/
    LEX_ID,                                 /*42*/
    POLIZ_LABEL,                            /*43*/
    POLIZ_ADDRESS,                          /*44*/
    POLIZ_GO,                               /*45*/
    POLIZ_FGO,                              /*46*/
    LV_STRING,                              /*47*/
    LV_INT,                                 /*48*/
    LV_BOOL                                 /*49*/
};

class Ident {
    std::string name;
    std::string string_value;
    bool declare;
    type_of_lex type;
    bool assign;
    int value;
public:
    Ident(): declare(false), assign(false) {}
    Ident(const std::string& new_str): name(new_str), declare(false), assign(false) {}
    bool operator==(const std::string& s) const { return name == s; }
    std::string get_name() const { return name; }
    void put_name(const std::string& new_name) { name = new_name; }
    bool get_declare() const { return declare; }
    void put_declare() { declare = true; }
    type_of_lex get_type() const { return type; }
    void put_type(type_of_lex t) { type = t; }
    bool get_assign() const { return assign; }
    void put_assign() { assign = true; }
    int get_value() const { return value; }
    void put_value(int val) { value = val; }
    std::string get_str_val() const { return string_value; }
    void put_str_val(const std::string& str) { string_value = str; }
};

class Lex {
    type_of_lex t_lex;
    int v_lex;
    std::string s_lex;
public:
    Lex(type_of_lex t = LEX_NULL, int v = 0): t_lex(t), v_lex(v), s_lex("") {}
    Lex(type_of_lex t, const std::string& str, int v = 0): t_lex(t), v_lex(v), s_lex(str) {}
    type_of_lex get_type() const { return t_lex; }
    int get_value() const { return v_lex; }
    std::string get_str() const { return s_lex; }
    void put_type(type_of_lex t) { t_lex = t; }
    friend std::ostream& operator<<(std::ostream& s, const Lex& l);
};

class Scanner {
    FILE *fp;
    char c;
    int search(const std::string& buf, const char **list);
    void gc();
public:
    int row, column;
    static int balance;
    bool st_start;
    static bool lex_flag;
    static const char *TW[];
    static const char *TD[];
    Scanner(const char *program);
    Lex get_lex();
    ~Scanner() { fclose(fp); }
};

int put(const std::string &buf);
extern std::vector<Ident> TID;

#endif
