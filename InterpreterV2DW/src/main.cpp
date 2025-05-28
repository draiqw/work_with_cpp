#include <iostream>
#include <string>
#include "parser.h"
#include "executor.h"
#include "lexer.h"

using namespace std;

class Interpretator {
    Parser pars;
    Executer E;
public:
    Interpretator(const char* program): pars(program) {}
    void interpretation() {
        pars.analyze();
        E.execute(pars.poliz);
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Дайте на вход имя файла" << endl;
        return 1;
    }
    try {
        Scanner scan(argv[1]);
        while (true) {
            try {
                Lex l = scan.get_lex();
                cout << l;
            } catch(string str) {
                if (str == "+++++++++++OK+++++++++++") {
                    Lex l2 = Lex(LEX_END, '}');
                    cout << l2;
                    cout << str << endl;
                    break;
                } else {
                    cout << str << endl;
                    return 0;
                }
            }
        }
    } catch(string str) {
        cout << str << endl;
        return 0;
    }
    try {
        Interpretator I(argv[1]);
        I.interpretation();
    } catch(string str) {
        cout << str;
        return 0;
    }
    return 0;
}
