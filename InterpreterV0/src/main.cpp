#include <iostream>
#include <fstream>
#include <string>
#include "lexer.h"
#include "parser.h"
#include "executor.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <program_file>" << std::endl;
        return 1;
    }
    std::ifstream fin(argv[1]);
    if (!fin.is_open()) {
        std::cerr << "Error: cannot open file " << argv[1] << std::endl;
        return 1;
    }
    // Read the entire program file into a string
    std::string text((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    fin.close();
    try {
        Lexer lexer(text);
        Parser parser(lexer);
        parser.analyze();                   // perform lexical and syntax analysis, build POLIZ
        Executer executer;
        executer.execute(parser.getPoliz()); // execute the POLIZ program
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
