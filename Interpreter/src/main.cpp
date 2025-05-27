#include <fstream>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: interpreter <source_file>\n";
        return 1;
    }
    const char* filename = argv[1];
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "Error: Cannot open file " << filename << "\n";
        return 1;
    }
    // Create Lexer and Parser
    Lexer lexer(infile);
    Parser parser(lexer);
    // Parse the program
    if (!parser.parseProgram()) {
        std::cerr << "Parsing failed. Exiting.\n";
        return 1;
    }
    // Get the generated code and symbol table
    const std::vector<Instruction>& code = parser.getCode();
    const std::vector<VarInfo>& vars = parser.getSymbolTable();
    // Run the interpreter on the generated code
    Interpreter interpreter(code, vars);
    interpreter.run();
    return 0;
}
