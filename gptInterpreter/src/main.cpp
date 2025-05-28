
#include "interpreter.h"
#include <iostream>

using namespace interpretator_model;

int main(int argc, char** argv) {
    const char* filename = (argc > 1) ? argv[1] : "data/testProg1.txt";
    Interpretator interpreter(filename);
    interpreter.analyze();
    return 0;
}
