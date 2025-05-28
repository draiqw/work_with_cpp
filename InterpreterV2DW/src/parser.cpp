#include "parser.h"
#include "poliz.h"
#include "executor.h"
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <string>

using namespace std;

vector<My_struct> structs;
vector<Struct_info> info;

int checker(const string &name) {
    for (int it = 0; it < (int)structs.size(); ++it) {
        if (structs[it].struct_name == name) {
            return it;
        }
    }
    cout << name << endl;
    string s = "Этот идентификатор не является структурой";
    throw s;
}

Parser::Parser(const char* program): scan(program), end(0) {}

void Parser::gl() // считать очередную лексему и информацию о ней
{
    curr_lex = scan.get_lex();
    c_type = curr_lex.get_type();
    c_val = curr_lex.get_value();
    c_str = curr_lex.get_str();
}

void Parser::analyze() // запуск рекурсивного спуска
{
    gl();
    P();
    if (c_type != LEX_END || end != 0) {
        cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Не хватает закрывающей скобки. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
    for (Lex l : poliz)
        cout << l;
    cout << "OK" << endl;
}

void Parser::P() // грамматика: program { <S;><C;> B }
{
    if (c_type == LEX_PROGRAM) {
        gl();
    } else {
        cout << "Ошибка. Имя правила: P, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Программа должна начинаться со слова program. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
    if (c_type == LEX_BEGIN) {
        gl();
        end++;
    } else {
        cout << "Ошибка. Имя правила: P, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): После слова program должна идти открывающая фигурная скобка. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
    while (c_type == TYPE_STRUCT) {
        S();  // описание структуры
        gl();
    }
    init_analyze();  // описание переменных в начальном блоке
    B();
    if ((c_type == LEX_END) && (end == 1)) {
        end--;
    }
}

void Parser::init_analyze() {
    if (c_type == LEX_INT || c_type == LEX_BOOL || c_type == LEX_STRING) {
        while (c_type == LEX_INT || c_type == LEX_BOOL || c_type == LEX_STRING) {
            type = c_type;
            gl();
            C();
            if (c_type == LEX_SEMICOLON) {
                gl();
            } else {
                cout << "Ошибка. Имя правила: P, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): Каждое описание должно заканчиваться ';'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                string str = string(s);
                throw str;
            }
        }
    } else if (c_type == LEX_ID) {
        // Возможен случай, когда идентификатор является именем структуры
        int row = checker(c_str);
        gl();
        if (c_type == LEX_ID) {
            string cur_id = curr_lex.get_str();
            gl();
            if (c_type == LEX_SEMICOLON) {
                int value;
                // добавить переменные структуры в таблицу идентификаторов
                for (int i = 0; i < structs[row].STID.size(); i++) {
                    string name_field = cur_id + "." + structs[row].STID[i].get_name();
                    value = put(name_field);
                    TID[value].put_type(structs[row].STID[i].get_type());
                    TID[value].put_declare();
                }
                gl();
            } else if (c_type == LEX_COMMA) {
                // несколько переменных структурного типа
                string struct_name = cur_id;
                int value;
                while (c_type == LEX_COMMA) {
                    gl();
                    if (c_type != LEX_ID) {
                        cout << "Ошибка. Имя правила: init_analyze, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                        char s[BUFSIZE];
                        sprintf(s, "%s (%d): Ожидался идентификатор после ','. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                        string str = string(s);
                        throw str;
                    }
                    string new_id = curr_lex.get_str();
                    for (int i = 0; i < structs[row].STID.size(); i++) {
                        string name_field = new_id + "." + structs[row].STID[i].get_name();
                        value = put(name_field);
                        TID[value].put_type(structs[row].STID[i].get_type());
                        TID[value].put_declare();
                    }
                    gl();
                }
                if (c_type == LEX_SEMICOLON) {
                    gl();
                } else {
                    cout << "Ошибка. Имя правила: init_analyze, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                    char s[BUFSIZE];
                    sprintf(s, "%s (%d): Описание переменных структурного типа должно заканчиваться ';'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                    string str = string(s);
                    throw str;
                }
            } else {
                cout << "Ошибка. Имя правила: init_analyze, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): Неправильный синтаксис описания переменной структурного типа. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                string str = string(s);
                throw str;
            }
        } else {
            cout << "Ошибка. Имя правила: init_analyze, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Ожидался идентификатор после имени структуры. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    }
}

void Parser::S() {
    // Описание структуры: struct <id> { <описание полей> }
    gl();
    if (c_type != LEX_ID) {
        cout << "Ошибка. Имя правила: S, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Ожидалось имя структуры после 'struct'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
    string struct_name = c_str;
    My_struct new_struct;
    new_struct.struct_name = struct_name;
    gl();
    if (c_type != LEX_BEGIN) {
        cout << "Ошибка. Имя правила: S, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Ожидалась '{' после имени структуры. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
    gl();
    // Считываем описания полей структуры
    while (c_type == LEX_INT || c_type == LEX_BOOL || c_type == LEX_STRING) {
        type = c_type;
        gl();
        H();
        if (c_type == LEX_SEMICOLON) {
            gl();
        } else {
            cout << "Ошибка. Имя правила: S, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Описание поля структуры должно заканчиваться ';'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    }
    if (c_type == LEX_END) {
        // сохранить описание структуры
        structs.push_back(new_struct);
        gl();
    } else {
        cout << "Ошибка. Имя правила: S, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Ожидался '}' в конце описания структуры. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }

    // вспомогательная функция для добавления полей в структуру
    info.clear();
    for (Ident id : new_struct.STID) {
        Struct_info si;
        si.st_type = struct_name;
        si.st_name = id.get_name();
        info.push_back(si);
    }
}

void Parser::H() {
    // Описание одного поля структуры: тип <id>{,<id>}
    if (c_type != LEX_ID) {
        cout << "Ошибка. Имя правила: H, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Ожидалось имя поля структуры. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
    // Добавить идентификатор поля в текущую структуру
    Ident ident(c_str);
    ident.put_type(type);
    ident.put_declare();
    structs.back().STID.push_back(ident);
    gl();
    while (c_type == LEX_COMMA) {
        gl();
        if (c_type != LEX_ID) {
            cout << "Ошибка. Имя правила: H, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Ожидалось имя поля после ','. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        Ident ident2(c_str);
        ident2.put_type(type);
        ident2.put_declare();
        structs.back().STID.push_back(ident2);
        gl();
    }
}

void Parser::B() {
    // Тело программы: последовательность операторов до '}'
    while (c_type != LEX_END) {
        O();
    }
}

void Parser::O() {
    // Один оператор (операторы разделяются ';' вне блоков)
    if (c_type == LEX_IF) {
        gl();
        if (c_type == LEX_LPAREN) {
            gl();
        } else {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После if должна идти '('. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        E();
        if (c_type == LEX_RPAREN) {
            gl();
        } else {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Ожидалась ')'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        int pl1 = poliz.size();
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_FGO));
        O();
        int pl2 = poliz.size();
        poliz[pl1] = Lex(POLIZ_LABEL, pl2);
        if (c_type == LEX_ELSE) {
            gl();
            O();
        }
    } else if (c_type == LEX_WHILE) {
        int pl0 = poliz.size();
        cycle_flag = true;
        gl();
        if (c_type == LEX_LPAREN) {
            gl();
        } else {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После while должна идти '('. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        E();
        if (c_type == LEX_RPAREN) {
            gl();
        } else {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Ожидалась ')'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        int pl3 = poliz.size();
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_FGO));
        int pl4 = poliz.size();
        B();
        poliz.push_back(Lex(POLIZ_LABEL, pl0));
        poliz.push_back(Lex(POLIZ_GO));
        poliz[pl3] = Lex(POLIZ_LABEL, poliz.size());
        cycle_flag = false;
    } else if (c_type == LEX_FOR) {
        cycle_flag = true;
        gl();
        if (c_type == LEX_LPAREN) {
            gl();
        } else {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После слова for должна идти открывающая скобка. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        if (c_type != LEX_SEMICOLON) {
            E();
            if (c_type == LEX_SEMICOLON) {
                gl();
            } else {
                cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): Обязательно должна идти ';'. Строка %d, столбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                string str = string(s);
                throw str;
            }
        } else {
            gl();
        }
        int pl6 = poliz.size();
        if (c_type != LEX_SEMICOLON) {
            E();
            if (c_type == LEX_SEMICOLON) {
                gl();
            } else {
                cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): Обязательно должна идти ';'. Строка %d, столбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                string str = string(s);
                throw str;
            }
        } else {
            gl();
        }
        int pl4 = poliz.size();
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_FGO));
        int pl5 = poliz.size();
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_GO));
        int pl7 = poliz.size();
        if (c_type != LEX_SEMICOLON && c_type != LEX_RPAREN) {
            E();
            if (c_type == LEX_RPAREN) {
                gl();
            } else {
                cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): После всех аргументов for должна идти закрывающая скобка. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                string str = string(s);
                throw str;
            }
        } else {
            gl();
        }
        poliz[pl5] = Lex(POLIZ_LABEL, poliz.size());
        B();
        poliz.push_back(Lex(POLIZ_LABEL, pl7));
        poliz.push_back(Lex(POLIZ_GO));
        poliz[pl4] = Lex(POLIZ_LABEL, poliz.size());
        cycle_flag = false;
    } else if (c_type == LEX_BREAK) {
        if (!cycle_flag) {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Оператор break не может использоваться вне цикла. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        } else {
            break_flag = true;
            int pl = poliz.size();
            poliz.push_back(Lex());
            poliz.push_back(Lex(POLIZ_GO));
            ints_stack.push(pl);
        }
        gl();
        if (c_type == LEX_SEMICOLON) {
            gl();
        } else {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После break должна идти ';'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    } else if (c_type == LEX_GOTO) {
        gl();
        if (c_type == LEX_ID) {
            int buf1 = new_goto_label(c_val, 0);
            if (buf1 != 0) {
                poliz.push_back(Lex(POLIZ_LABEL, buf1));
                poliz.push_back(Lex(POLIZ_GO));
            } else {
                new_goto_label(c_val, int(poliz.size()));
                poliz.push_back(Lex());
                poliz.push_back(Lex(POLIZ_GO));
            }
            gl();
            if (c_type == LEX_SEMICOLON) {
                gl();
            } else {
                cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): После goto должна идти ';'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                string str = string(s);
                throw str;
            }
        } else {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Ошибка метки в goto. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    } else if (c_type == LEX_READ) {
        gl();
        if (c_type == LEX_LPAREN) {
            gl();
        } else {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После слова read должна идти открывающая скобка. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        if (c_type == LEX_ID) {
            check_id_in_read();
            poliz.push_back(Lex(POLIZ_ADDRESS, c_val));
            gl();
        } else {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): В операторе read может стоять только идентификатор. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        if (c_type == LEX_RPAREN) {
            gl();
        } else {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Ожидалась ')'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        poliz.push_back(Lex(LEX_READ));
        if (c_type == LEX_SEMICOLON) {
            gl();
        } else {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После оператора read должна идти ';'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    } else if (c_type == LEX_WRITE) {
        gl();
        if (c_type == LEX_LPAREN) {
            gl();
        } else {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После слова write должна идти '('. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        E();
        while (c_type == LEX_COMMA) {
            gl();
            E();
        }
        if (c_type == LEX_RPAREN) {
            gl();
        } else {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Ожидалась ')'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        poliz.push_back(Lex(LEX_WRITE));
        if (c_type == LEX_SEMICOLON) {
            gl();
        } else {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После оператора write должна идти ';'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    } else if (c_type == LEX_BEGIN) {
        gl();
        end++;
        B();
        if (c_type == LEX_END && end > 0) {
            end--;
            gl();
        } else {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Не найден соответствующий '}'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    } else if (c_type == LEX_SEMICOLON) {
        gl();
    } else {
        // Обработка выражения или присваивания
        E();
        if (c_type == LEX_SEMICOLON) {
            poliz.push_back(Lex(LEX_SEMICOLON));
            gl();
        } else {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Пропущен ';'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    }
    if (break_flag) {
        // Patching break jump out of loop
        int pl_index = ints_stack.top();
        ints_stack.pop();
        poliz[pl_index] = Lex(POLIZ_LABEL, poliz.size());
        break_flag = false;
    }
}

void Parser::O1() {
    // Обработка необязательной части оператора if (после else)
    O();
}

void Parser::E() {
    // Обработка выражения
    E1();
    while (c_type == LEX_OR) {
        type_of_lex tmp_type = c_type;
        gl();
        E1();
        check_op(scan.row, scan.column);
        poliz.push_back(Lex(tmp_type));
    }
}

void Parser::E1() {
    T();
    while (c_type == LEX_AND) {
        type_of_lex tmp_type = c_type;
        gl();
        T();
        check_op(scan.row, scan.column);
        poliz.push_back(Lex(tmp_type));
    }
}

void Parser::T() {
    F();
    while (c_type == LEX_EQ || c_type == LEX_LSS || c_type == LEX_GTR ||
           c_type == LEX_LEQ || c_type == LEX_GEQ || c_type == LEX_NEQ) {
        type_of_lex op = c_type;
        gl();
        F();
        check_op(scan.row, scan.column);
        poliz.push_back(Lex(op));
    }
}

void Parser::F() {
    J();
    while (c_type == LEX_PLUS || c_type == LEX_MINUS || c_type == LEX_PERCENT ||
           c_type == LEX_TIMES || c_type == LEX_SLASH) {
        type_of_lex op = c_type;
        gl();
        J();
        check_op(scan.row, scan.column);
        poliz.push_back(Lex(op));
    }
}

void Parser::L() {
    // Обработка присваивания
    if (c_type != LEX_ID) {
        cout << "Ошибка. Имя правила: L, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Ожидался идентификатор в левой части присваивания. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
    check_id();
    st_lex.push(TID[c_val].get_type());
    int cur_val = c_val;
    poliz.push_back(Lex(POLIZ_ADDRESS, c_val));
    gl();
    if (c_type == LEX_ASSIGN) {
        gl();
        E();
        eq_type(scan.row, scan.column);
        poliz.push_back(Lex(LEX_ASSIGN));
    } else {
        cout << "Ошибка. Имя правила: L, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Ожидался символ '=' после идентификатора. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
    TID[cur_val].put_assign();
}

void Parser::K() {
    // Обработка функции или вызова без присваивания
    if (c_type == LEX_ID) {
        if (!TID[c_val].get_declare()) {
            // Проверка: если идентификатор не был объявлен
            cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Идентификатор не описан. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        poliz.push_back(curr_lex);
        st_lex.push(TID[c_val].get_type());
        gl();
    } else if (c_type == LEX_NOT) {
        int tmp_r = scan.row;
        int tmp_c = scan.column;
        gl();
        K();
        check_not(tmp_r, tmp_c);
    } else if (c_type == LEX_LPAREN) {
        gl();
        E();
        if (c_type == LEX_RPAREN) {
            gl();
        } else {
            cout << "Ошибка. Имя правила: K, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Ожидалась ')'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    } else {
        NN();
    }
}

void Parser::J() {
    // Обработка унарных операций и идентификаторов
    if (c_type == LEX_ID) {
        if (!TID[c_val].get_declare()) {
            // Если идентификатор не объявлен, возможна ситуация метки
            int buf7 = c_val;
            gl();
            if (c_type == LEX_COLON) {
                lb_flag = true;
                new_label(buf7);
                int buf8;
                if ((buf8 = new_goto_label(buf7, int(poliz.size()))) != 0) {
                    int buf9 = int(poliz.size());
                    poliz[buf8] = Lex(POLIZ_LABEL, buf9);
                }
                gl();
                O();
            } else {
                cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): Идентификатор не описан. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                string str = string(s);
                throw str;
            }
        } else {
            poliz.push_back(curr_lex);
            st_lex.push(TID[c_val].get_type());
            gl();
        }
    } else if (c_type == LEX_NOT) {
        int tmp_r = scan.row;
        int tmp_c = scan.column;
        gl();
        J();
        check_not(tmp_r, tmp_c);
    } else if (c_type == LEX_LPAREN) {
        gl();
        E();
        if (c_type == LEX_RPAREN) {
            gl();
        } else {
            cout << "Ошибка. Имя правила: J, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Ожидалась закрывающая скобка. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    } else {
        NN();
    }
}

void Parser::NN() {
    // Обработка отрицательного числа (унарный минус перед числом)
    if (c_type == LEX_MINUS) {
        gl();
        if (c_type == LEX_NUM) {
            type_const = c_type;
            st_lex.push(LEX_INT);
            poliz.push_back(Lex(curr_lex.get_type(), -curr_lex.get_value()));
            gl();
        } else {
            cout << "Ошибка. Имя правила: NN, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Ожидалось число. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    } else if (c_type == LEX_NUM) {
        type_const = c_type;
        st_lex.push(LEX_INT);
        poliz.push_back(curr_lex);
        gl();
    } else if (c_type == LEX_TRUE) {
        type_const = c_type;
        st_lex.push(LEX_BOOL);
        poliz.push_back(Lex(LEX_TRUE, 1));
        gl();
    } else if (c_type == LEX_FALSE) {
        type_const = c_type;
        st_lex.push(LEX_BOOL);
        poliz.push_back(Lex(LEX_FALSE, 0));
        gl();
    } else if (c_type == LEX_STR) {
        type_const = c_type;
        st_lex.push(LEX_STRING);
        poliz.push_back(Lex(LEX_STR, c_str));
        gl();
    } else {
        cout << "Ошибка. Имя правила: NN, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Ожидался операнд. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
}

void Parser::dec(type_of_lex t) {
    // Отметить идентификатор как объявленный
    if (TID[c_val].get_declare()) {
        cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Повторное описание идентификатора. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    } else {
        TID[c_val].put_declare();
        TID[c_val].put_type(t);
    }
}

void Parser::check_init(type_of_lex t, type_of_lex t_c) {
    if (t == LEX_STRING && t_c == LEX_STR) return;
    if ((t == LEX_INT && (t_c == LEX_INT || t_c == LEX_NUM)) ||
        (t == LEX_BOOL && (t_c == LEX_BOOL || t_c == LEX_TRUE || t_c == LEX_FALSE)) ||
        (t == LEX_STRING && t_c == LEX_STRING)) {
        return;
    }
    cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
    char s[BUFSIZE];
    sprintf(s, "%s (%d): Несовместимая инициализация переменной. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
    string str = string(s);
    throw str;
}

void Parser::check_id() {
    if (!TID[c_val].get_declare()) {
        // Использование необъявленного идентификатора
        cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Идентификатор не описан. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
}

void Parser::check_op(int r, int c) {
    type_of_lex t2, op, t1;
    from_st(st_lex, t2);
    from_st(st_lex, t1);
    from_st(st_lex, op);
    if (op == LEX_PLUS || op == LEX_MINUS || op == LEX_TIMES || op == LEX_SLASH || op == LEX_PERCENT) {
        if ((t1 == LEX_INT || t1 == LV_INT) && (t2 == LEX_INT || t2 == LV_INT)) {
            st_lex.push(LEX_INT);
        } else if ((t1 == LEX_STRING || t1 == LV_STRING) && (t2 == LEX_STRING || t2 == LV_STRING) && op == LEX_PLUS) {
            st_lex.push(LEX_STRING);
        } else {
            cout << "Ошибка. Тип лексемы: " << op << ". Несовместимые операнды. Строка " << r << ", cтолбец " << c << endl;
            throw string("Типовая ошибка операций");
        }
    } else if (op == LEX_OR || op == LEX_AND) {
        if ((t1 == LEX_BOOL || t1 == LV_BOOL) && (t2 == LEX_BOOL || t2 == LV_BOOL)) {
            st_lex.push(LEX_BOOL);
        } else {
            cout << "Ошибка. Тип лексемы: " << op << ". Логические операнды несовместимы. Строка " << r << ", cтолбец " << c << endl;
            throw string("Типовая ошибка логических операций");
        }
    } else {
        // Операции сравнения
        if ((t1 == LEX_INT || t1 == LV_INT || t1 == LEX_STRING || t1 == LV_STRING) &&
            (t2 == LEX_INT || t2 == LV_INT || t2 == LEX_STRING || t2 == LV_STRING)) {
            st_lex.push(LEX_BOOL);
        } else {
            cout << "Ошибка. Тип лексемы: " << op << ". Неверные типы для операции сравнения. Строка " << r << ", cтолбец " << c << endl;
            throw string("Типовая ошибка операции сравнения");
        }
    }
}

void Parser::check_not(int r, int c) {
    if (st_lex.top() != LEX_BOOL && st_lex.top() != LV_BOOL) {
        cout << "Ошибка. Имя правила: check_not, обнаружен тип: " << st_lex.top();
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Оператор 'not' применяется к не-логическому выражению. Строка %d, cтолбец %d\n", __FILE__, __LINE__, r, c);
        string str = string(s);
        throw str;
    }
}

void Parser::eq_type(int r, int c) {
    type_of_lex t2, t1;
    from_st(st_lex, t2);
    from_st(st_lex, t1);
    if ((t1 == LEX_INT && (t2 == LEX_INT || t2 == LV_INT || t2 == LEX_NUM)) ||
        (t1 == LEX_BOOL && (t2 == LEX_BOOL || t2 == LV_BOOL)) ||
        (t1 == LEX_STRING && (t2 == LEX_STRING || t2 == LV_STRING || t2 == LEX_STR))) {
        st_lex.push(t1);
    } else {
        cout << "Ошибка. Имя правила: eq_type, несовместимые типы: " << t1 << " и " << t2;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Несовместимые типы в присваивании. Строка %d, cтолбец %d\n", __FILE__, __LINE__, r, c);
        string str = string(s);
        throw str;
    }
}

void Parser::eq_bool(int r, int c) {
    if (st_lex.top() != LEX_BOOL) {
        cout << "Ошибка. Имя правила: eq_bool, тип лексемы: " << st_lex.top();
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Выражение не является логическим. Строка %d, cтолбец %d\n", __FILE__, __LINE__, r, c);
        string str = string(s);
        throw str;
    }
    st_lex.pop();
}

void Parser::check_id_in_read() {
    if (!TID[c_val].get_declare()) {
        cout << "Ошибка. Имя правила: check_id_in_read, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Идентификатор не описан перед использованием в read. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
}
