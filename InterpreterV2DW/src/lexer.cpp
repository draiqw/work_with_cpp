#include "lexer.h"

using namespace std;

vector<Ident> TID;
int Scanner::balance = 0;
bool Scanner::lex_flag = true;
const char * Scanner::TW[] = { "", "and",  "bool", "do", "else",  "if", "false", "int", "not", "or", "program",
                               "read", "string", "true", "goto", "break", "while", "write", "for", "struct",  NULL };
const char * Scanner::TD[] = { "\"", "{", "}", ";", ",", ":", "!=", "=", "(", ")",
                               "==", "<", ">", "+", "-", "*", "/", "<=", ">=", "%", NULL };

Scanner::Scanner(const char *program): row(1), column(0), st_start(false) {
    if (!(fp = fopen(program, "r"))) {
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Не удалось открыть файл\n", __FILE__, __LINE__);
        string str = string(s);
        throw str;
    }
}

int Scanner::search(const std::string& buf, const char **list) {
    int i = 0;
    while (list[i]) {
        if (buf == list[i]) {
            return i;
        }
        ++i;
    }
    return 0;
}

void Scanner::gc() {
    c = fgetc(fp);
    if ((c == EOF) && (balance != 0)) {
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Не хватает закрывающей скобки. Строка %d, cтолбец %d\n", __FILE__, __LINE__, row, column);
        string str = string(s);
        throw str;
    }
}

Lex Scanner::get_lex() {
    enum state { H, IDENT, ST_IDENT, NUMB, SLASH, COM, COM_TIMES, ALE, NEQ, STR, START, FINISH };
    int d;
    int j;
    string buf;
    string buf1;
    state CS = H;
    do {
        column++;
        gc();
        switch (CS) {
            case H:
                if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
                    if (c == '\n') {
                        column = 0;
                        row++;
                    }
                } else if (c == '{') {
                    balance++;
                    return Lex(LEX_BEGIN, '{');
                } else if (c == '}') {
                    if (st_start) {
                        st_start = false;
                    }
                    balance--;
                    if (balance < 0) {
                        char s[BUFSIZE];
                        sprintf(s, "%s (%d): Непонятная закрывающая скобка. Строка %d, cтолбец %d\n", __FILE__, __LINE__, row, column);
                        string str = string(s);
                        throw str;
                    }
                    if ((balance == 0) && (lex_flag)) {
                        string s = "+++++++++++OK+++++++++++";
                        lex_flag = false;
                        throw s;
                    }
                    return Lex(LEX_END, '}');
                } else if (islower(c)) {
                    buf.push_back(c);
                    CS = IDENT;
                } else if (isdigit(c)) {
                    d = c - '0';
                    CS = NUMB;
                } else if (c == '/') {
                    buf.push_back(c);
                    CS = SLASH;
                } else if (c == '=' || c == '<' || c == '>') {
                    buf.push_back(c);
                    CS = ALE;
                } else if (c == '!') {
                    buf.push_back(c);
                    CS = NEQ;
                } else if (c == '\"') {
                    CS = STR;
                } else {
                    buf.push_back(c);
                    if ((j = search(buf, TD))) {
                        return Lex((type_of_lex)(j + (int)LEX_QUOTE), j);
                    }
                }
                break;
            case IDENT:
                if (isalpha(c) || isdigit(c) || c == '.') {
                    buf.push_back(c);
                } else {
                    ungetc(c, fp);
                    column--;
                    if ((j = search(buf, TW))) {
                        return Lex((type_of_lex) j, j);
                    } else {
                        j = TID.size();
                        TID.push_back(Ident(buf));
                        return Lex(LEX_ID, j);
                    }
                }
                break;
            case NUMB:
                if (isdigit(c)) {
                    d = d * 10 + (c - '0');
                } else {
                    ungetc(c, fp);
                    column--;
                    return Lex(LEX_NUM, d);
                }
                break;
            case SLASH:
                if (c == '*') {
                    CS = COM;
                    buf.pop_back();
                } else {
                    ungetc(c, fp);
                    column--;
                    j = search(buf, TD);
                    return Lex(LEX_SLASH, j);
                }
                break;
            case COM:
                if (c == '*') {
                    CS = COM_TIMES;
                }
                break;
            case COM_TIMES:
                if (c == '/') {
                    CS = H;
                } else {
                    CS = COM;
                }
                break;
            case ALE:
                if (c == '=') {
                    buf.push_back(c);
                    j = search(buf, TD);
                    return Lex((type_of_lex)(j + (int)LEX_QUOTE), j);
                } else {
                    ungetc(c, fp);
                    column--;
                    j = search(buf, TD);
                    return Lex((type_of_lex)(j + (int)LEX_QUOTE), j);
                }
                break;
            case NEQ:
                if (c == '=') {
                    buf.push_back(c);
                    j = search(buf, TD);
                    return Lex(LEX_NEQ, j);
                } else {
                    char s[BUFSIZE];
                    sprintf(s, "%s (%d): После '!' должно идти '='. Строка %d, cтолбец %d\n", __FILE__, __LINE__, row, column);
                    string str = string(s);
                    throw str;
                }
                break;
            case STR:
                if (c != '\"') {
                    buf.push_back(c);
                } else {
                    return Lex(LEX_STR, buf);
                }
                break;
        }
    } while (true);
}

ostream& operator<<(ostream& s, const Lex& l) {
    string t;
    if (l.t_lex <= TYPE_STRUCT) {
        t = Scanner::TW[l.t_lex];
    } else if (l.t_lex >= LEX_QUOTE && l.t_lex <= LEX_PERCENT) {
        t = Scanner::TD[l.t_lex - LEX_QUOTE];
    } else if (l.t_lex == LEX_NUM) {
        t = "NUMB";
    } else if (l.t_lex == LEX_ID) {
        t = TID[l.v_lex].get_name();
    } else if (l.t_lex == LEX_STR) {
        s << "(STR," << l.s_lex << ");" << endl;
        return s;
    } else if (l.t_lex == POLIZ_LABEL) {
        t = "Label";
    } else if (l.t_lex == POLIZ_ADDRESS) {
        t = "Addr";
    } else if (l.t_lex == POLIZ_GO) {
        t = "!";
    } else if (l.t_lex == POLIZ_FGO) {
        t = "!F";
    } else if (l.t_lex == LEX_TRUE) {
        t = "true";
    } else if (l.t_lex == LEX_FALSE) {
        t = "false";
    } else if (l.t_lex == LEX_BEGIN) {
        t = "{";
    } else if (l.t_lex == LEX_END) {
        t = "}";
    } else if (l.t_lex == LEX_SEMICOLON) {
        t = ";";
    } else if (l.t_lex == LEX_COMMA) {
        t = ",";
    } else if (l.t_lex == LEX_COLON) {
        t = ":";
    } else if (l.t_lex == LEX_LPAREN) {
        t = "(";
    } else if (l.t_lex == LEX_RPAREN) {
        t = ")";
    } else if (l.t_lex == LEX_EQ) {
        t = "==";
    } else if (l.t_lex == LEX_LSS) {
        t = "<";
    } else if (l.t_lex == LEX_GTR) {
        t = ">";
    } else if (l.t_lex == LEX_PLUS) {
        t = "+";
    } else if (l.t_lex == LEX_MINUS) {
        t = "-";
    } else if (l.t_lex == LEX_TIMES) {
        t = "*";
    } else if (l.t_lex == LEX_SLASH) {
        t = "/";
    } else if (l.t_lex == LEX_LEQ) {
        t = "<=";
    } else if (l.t_lex == LEX_GEQ) {
        t = ">=";
    } else if (l.t_lex == LEX_NEQ) {
        t = "!=";
    } else if (l.t_lex == LEX_ASSIGN) {
        t = "=";
    } else if (l.t_lex == LEX_PERCENT) {
        t = "%";
    }
    s << "(" << t << "," << l.v_lex << ");" << endl;
    return s;
}

int put(const string &buf) {
    vector<Ident>::iterator k;
    if ((k = find(TID.begin(), TID.end(), buf)) != TID.end()) {
        return k - TID.begin();
    }
    TID.push_back(Ident(buf));
    return TID.size() - 1;
}
