#ifndef POLIZ_H
#define POLIZ_H

#include <vector>

extern std::vector<int> labels;
extern std::vector<int> address_label;
extern std::vector<int> gotos;

void new_label(int num);
int new_goto_label(const int buf, const int num);

#endif
